#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "client.h"

#define  BUFF_SIZE   153600
#define maxDistanceMM  12500
#define  MAX_PASE  30000

//char response[BUFF_SIZE *2];
unsigned char distance[BUFF_SIZE];
unsigned char intensity[BUFF_SIZE];

//char * response;
int width = 320;
int height = 252;
int dataPixelfield[252][320];
int finalDistance[252][320];




int calcDistance(int phase)
{
    if(phase < 0)
        return -1;

    int dis = maxDistanceMM * phase / MAX_PASE;

    return dis;   
}

int calcPixel(char *data , int i)
{
    unsigned char pixelMSB = (unsigned char) data[2*i+1];
    unsigned char pixelLSB = (unsigned char) data[2*i];

    int pixel = (((int)pixelMSB << 8) + pixelLSB);
    if (pixel < 8000){ // no special value like saturation, ADC overflow or too low amplitude
        return pixel ;
    }else {
        return pixel;
    }

}

int* GetDistanceAndAmplitudeSorted()
{
   int   client_socket;
   int nbyte = 256;

   struct sockaddr_in   server_addr;
   char *response = (char *)malloc(sizeof(char)*BUFF_SIZE*2);

   unsigned char   buff[1024];
   int test[252*320];
   //int *test = (int *)malloc(sizeof(int)*252*320);
   memset( &server_addr, 0, sizeof( server_addr));
   server_addr.sin_family     = AF_INET;
   server_addr.sin_port       = htons( 50660);
   server_addr.sin_addr.s_addr= inet_addr( "192.168.7.2");

   client_socket  = socket( PF_INET, SOCK_STREAM, 0);
   if( -1 == client_socket)
   {
      printf( "socket 생성 실패\n");
      exit( 1);
   }
    
   if( -1 == connect( client_socket, (struct sockaddr*)&server_addr,
                           sizeof( server_addr) ) )
   {
      printf( "접속 실패\n");
      exit( 1);
   }

   send( client_socket, "getDistanceAndAmplitudeSorted", strlen( "getDistanceAndAmplitudeSorted")+1, 0); // +1: NULL까지 포함해서 전송
   
    int sum = 0;
    nbyte = 256;
    while(nbyte!=0)
    {
        //printf("%d\n" , sum);
        nbyte = recv( client_socket, buff, 1024, 0);
        //if(sum < BUFF_SIZE)
        memcpy(response+sum , buff , sizeof(buff));
        sum += nbyte;
    }
    //printf("step 1\n");
    memcpy(distance , response , sizeof(distance));
    memcpy(intensity , response+BUFF_SIZE , sizeof(intensity));

   close( client_socket);
    
    int redY = 1;

    int c;
    int y , x;
    int xMax = -1 ;
    int yMax = -1;

   for(int i = 0 ; i < BUFF_SIZE/4 ; i++)
   {
        int pixel = calcPixel(distance, (int)(BUFF_SIZE / 4) - 1 - i);
        y = width - 1 - (int)(i % width) + 4;
        x = (int)(height / 2 - 1) - redY * (int)(i / width);
        
        dataPixelfield[y][x] = pixel;
        finalDistance[y][x] = calcDistance(pixel);
        
        c = y * 320 + x;
        
        pixel = calcPixel(distance, (int)(BUFF_SIZE / 4) + i);
        y = (int)(i % width) + 4;
        x = (int)(height / 2) + redY * (int)(i / width);
        
        dataPixelfield[y][x] = pixel;
        finalDistance[y][x] = calcDistance(pixel);
        
        c = y * 320 + x;
        
        //if(c > 320*252)
        //    printf("%d\n" , c);

   }

    // (242 , 111)
   // printf("%d\n" , finalDistance[242][111]);
 
    for(int i = 0 ; i < 252 ; i++)
        for(int j = 0 ; j < 320 ; j++)
        {
            int index = i * 320 + j;
            test[index] = finalDistance[i][j];
        }


    free(response);
    return test;
}
// int main( int argc, char **argv)
// {
    
//         int *arr ;
//         while(1)
//         {
//         arr = GetDistanceAndAmplitudeSorted();
//         printf("%d\n", arr[242 * 320 + 111]);

        
//         }
        
//         //free(arr);
//         //arr = GetDistanceAndAmplitudeSorted();
//         //printf("%d\n", arr[242 * 320 + 111]);
//       //free(brr);
//    return 0;
// }