#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "sqlite3.h"

/* Structure to store RFID data */
typedef struct
{
    char raw[64];   // raw data from STM (UID:XXXXXXXX\n)
    char uid[16];   // cleaned UID (XXXXXXXX)
} rfid_packet;

/* Function to extract UID from received data */
void parse_rfid(rfid_packet *pkt)
{
    /* remove newline */
    pkt->raw[strcspn(pkt->raw, "\r\n")] = '\0';

    /* remove UID: prefix */
    if (strncmp(pkt->raw, "UID:", 4) == 0)
        strcpy(pkt->uid, pkt->raw + 4);
    else
        strcpy(pkt->uid, pkt->raw);
}

/* Function to check RFID in database */
int check_rfid(sqlite3 *db, char *uid)
{
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT 1 FROM rfid_users WHERE rfid_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    sqlite3_bind_text(stmt, 1, uid, -1, SQLITE_STATIC);

    int found = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return found;
}

int main()
{
    int sockfd;
    struct sockaddr_in bbb_addr, stm_addr;
    socklen_t addr_len = sizeof(stm_addr);

    sqlite3 *db;
    rfid_packet pkt;

    /* Open database */
    if (sqlite3_open("allowed_ids.db", &db) != SQLITE_OK)
    {
        printf("Failed to open database\n");
        return 1;
    }

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bbb_addr.sin_family = AF_INET;
    bbb_addr.sin_port = htons(5005);
    bbb_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&bbb_addr, sizeof(bbb_addr));

    printf("BBB listening on UDP port 5005...\n");

    while (1)
    {
        int n = recvfrom(sockfd, pkt.raw, sizeof(pkt.raw) - 1,
                         0, (struct sockaddr *)&stm_addr, &addr_len);

        pkt.raw[n] = '\0';

        printf("RFID received: %s\n", pkt.raw);

        parse_rfid(&pkt);

        printf("Checking UID: %s\n", pkt.uid);

        if (check_rfid(db, pkt.uid))
        {
            sendto(sockfd, "ACK\n", 4, 0,
                   (struct sockaddr *)&stm_addr, addr_len);
            printf("→ ACK sent\n");
        }
        else
        {
            sendto(sockfd, "DENY\n", 5, 0,
                   (struct sockaddr *)&stm_addr, addr_len);
            printf("→ DENY sent\n");
        }
    }

    sqlite3_close(db);
    close(sockfd);
    return 0;
}

