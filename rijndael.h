/* rijndael.h 
 *
 * drt@ailis.de
 *
 * $id$ 
 *
 * $Log: rijndael.h,v $
 * Revision 1.1  2000/04/30 15:59:26  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.1.1.1  2000/04/16 17:38:10  drt
 * initial ddns version
 *
 * Revision 1.1.1.1  2000/04/12 16:07:17  drt
 * initial revision
 *
 */

/* Key Scheduler. Create expanded encryption key */
/* blocksize=32*nb bits. Key=32*nk bits */
/* currently nb,bk = 4, 6 or 8          */
/* key comes as 4*Nk bytes              */
void rijndaelKeySched(int nb, int nk, char *key);

void rijndaelEncrypt(char *buff);
void rijndaelDecrypt(char *buff);
