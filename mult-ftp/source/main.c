#include "fileopt.h"

int main()
{
    MFTP_ROLE_T role = MFTP_ROLE_SERVER;

    if (MFTP_ROLE_SERVER == role)
    {
        mftp_server_init();

        mftp_server_loop();

        mftp_server_uninit();
    }
    else
    {
        mftp_client_init();

        mftp_client_loop();

        mftp_client_uninit();
    }

    return 0;
}
