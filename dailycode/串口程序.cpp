#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<assert.h>
#include<termios.h>
#include<string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<errno.h>

static int ret;
static int fd;

/*
 * ��ȫ��д����
 */

ssize_t safe_write(int fd, const void *vptr, size_t n)
{
    size_t  nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0)
    {
    if((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if(nwritten < 0&&errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}

ssize_t safe_read(int fd,void *vptr,size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr=vptr;
    nleft=n;

    while(nleft > 0)
    {
        if((nread = read(fd,ptr,nleft)) < 0)
        {
            if(errno == EINTR)//���ź��ж�
                nread = 0;
            else
                return -1;
        }
        else
        if(nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    }
    return (n-nleft);
}

int uart_open(int fd,const char *pathname)
{
    assert(pathname);

    /*�򿪴���*/
    fd = open(pathname,O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd == -1)
    {
        perror("Open UART failed!");
        return -1;
    }

    /*������ڷ�������־*/
    if(fcntl(fd,F_SETFL,0) < 0)
    {
        fprintf(stderr,"fcntl failed!\n");
        return -1;
    }

    return fd;
}

int uart_set(int fd,int baude,int c_flow,int bits,char parity,int stop)
{
    struct termios options;

    /*��ȡ�ն�����*/
    if(tcgetattr(fd,&options) < 0)
    {
        perror("tcgetattr error");
        return -1;
    }


    /*����������������ʣ����߱���һ��*/
    switch(baude)
    {
        case 4800:
            cfsetispeed(&options,B4800);
            cfsetospeed(&options,B4800);
            break;
        case 9600:
            cfsetispeed(&options,B9600);
            cfsetospeed(&options,B9600);
            break;
        case 19200:
            cfsetispeed(&options,B19200);
            cfsetospeed(&options,B19200);
            break;
        case 38400:
            cfsetispeed(&options,B38400);
            cfsetospeed(&options,B38400);
            break;
        default:
            fprintf(stderr,"Unkown baude!\n");
            return -1;
    }

    /*���ÿ���ģʽ*/
    options.c_cflag |= CLOCAL;//��֤����ռ�ô���
    options.c_cflag |= CREAD;//��֤������ԴӴ����ж�ȡ����

    /*��������������*/
    switch(c_flow)
    {
        case 0://������������
            options.c_cflag &= ~CRTSCTS;
            break;
        case 1://����Ӳ��������
            options.c_cflag |= CRTSCTS;
            break;
        case 2://�������������
            options.c_cflag |= IXON|IXOFF|IXANY;
            break;
        default:
            fprintf(stderr,"Unkown c_flow!\n");
            return -1;
    }

    /*��������λ*/
    switch(bits)
    {
        case 5:
            options.c_cflag &= ~CSIZE;//����������־λ
            options.c_cflag |= CS5;
            break;
        case 6:
            options.c_cflag &= ~CSIZE;//����������־λ
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag &= ~CSIZE;//����������־λ
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag &= ~CSIZE;//����������־λ
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr,"Unkown bits!\n");
            return -1;
    }

    /*����У��λ*/
    switch(parity)
    {
        /*����żУ��λ*/
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;//PARENB��������żλ��ִ����żУ��
            options.c_cflag &= ~INPCK;//INPCK��ʹ��żУ��������
            break;
        /*��Ϊ�ո�,��ֹͣλΪ2λ*/
        case 's':
        case 'S':
            options.c_cflag &= ~PARENB;//PARENB��������żλ��ִ����żУ��
            options.c_cflag &= ~CSTOPB;//CSTOPB��ʹ����λֹͣλ
            break;
        /*������У��*/
        case 'o':
        case 'O':
            options.c_cflag |= PARENB;//PARENB��������żλ��ִ����żУ��
            options.c_cflag |= PARODD;//PARODD����������Ϊ��У��,����ΪżУ��
            options.c_cflag |= INPCK;//INPCK��ʹ��żУ��������
            options.c_cflag |= ISTRIP;//ISTRIP������������Ч�������ֱ�����7���ֽڣ�������ȫ��8λ
            break;
        /*����żУ��*/
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;//PARENB��������żλ��ִ����żУ��
            options.c_cflag &= ~PARODD;//PARODD����������Ϊ��У��,����ΪżУ��
            options.c_cflag |= INPCK;//INPCK��ʹ��żУ��������
            options.c_cflag |= ISTRIP;//ISTRIP������������Ч�������ֱ�����7���ֽڣ�������ȫ��8λ
            break;
        default:
            fprintf(stderr,"Unkown parity!\n");
            return -1;
    }

    /*����ֹͣλ*/
    switch(stop)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;//CSTOPB��ʹ����λֹͣλ
            break;
        case 2:
            options.c_cflag |= CSTOPB;//CSTOPB��ʹ����λֹͣλ
            break;
        default:
            fprintf(stderr,"Unkown stop!\n");
            return -1;
    }

    /*�������ģʽΪԭʼ���*/
    options.c_oflag &= ~OPOST;//OPOST���������򰴶�������������������c_oflagʧЧ

    /*���ñ���ģʽΪԭʼģʽ*/
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    /*
     *ICANON������淶ģʽ�������봦��
     *ECHO�����������ַ��ı��ػ���
     *ECHOE���ڽ���EPASEʱִ��Backspace,Space,Backspace���
     *ISIG�������ź�
     */

    /*���õȴ�ʱ�����С�����ַ�*/
    options.c_cc[VTIME] = 0;//������select������
    options.c_cc[VMIN] = 1;//���ٶ�ȡһ���ַ�

    /*����������������ֻ�������ݣ����ǲ����ж�����*/
    tcflush(fd,TCIFLUSH);

    /*��������*/
    if(tcsetattr(fd,TCSANOW,&options) < 0)
    {
        perror("tcsetattr failed");
        return -1;
    }

    return 0;
}

int uart_read(int fd,char *r_buf,size_t len)
{
    ssize_t cnt = 0;
    fd_set rfds;
    struct timeval time;

    /*���ļ����������������������*/
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);

    /*���ó�ʱΪ15s*/
    time.tv_sec = 15;
    time.tv_usec = 0;

    /*ʵ�ִ��ڵĶ�·I/O*/
    ret = select(fd+1,&rfds,NULL,NULL,&time);
    switch(ret)
    {
        case -1:
            fprintf(stderr,"select error!\n");
            return -1;
        case 0:
            fprintf(stderr,"time over!\n");
            return -1;
        default:
            cnt = safe_read(fd,r_buf,len);
            if(cnt == -1)
            {
                fprintf(stderr,"read error!\n");
                return -1;
            }
            return cnt;
    }
}

int uart_write(int fd,const char *w_buf,size_t len)
{
    ssize_t cnt = 0;

    cnt = safe_write(fd,w_buf,len);
    if(cnt == -1)
    {
        fprintf(stderr,"write error!\n");
        return -1;
    }

    return cnt;
}

int uart_close(int fd)
{
    assert(fd);
    close(fd);

    /*������������Щ������*/

    return 0;
}

int main(void)
{
    const char *w_buf = "something to write";
    size_t w_len = sizeof(w_buf);

    char r_buf[1024];
    bzero(r_buf,1024);

    fd = uart_open(fd,"/dev/ttyS0");/*���ں�/dev/ttySn,USB�ں�/dev/ttyUSBn*/
    if(fd == -1)
    {
        fprintf(stderr,"uart_open error\n");
        exit(EXIT_FAILURE);
    }

    if(uart_set(fd,9600,0,8,'N',1) == -1)
    {
        fprintf(stderr,"uart set failed!\n");
        exit(EXIT_FAILURE);
    }

    ret = uart_write(fd,w_buf,w_len);
    if(ret == -1)
    {
        fprintf(stderr,"uart write failed!\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        ret = uart_read(fd,r_buf,1024);
        if(ret == -1)
        {
            fprintf(stderr,"uart read failed!\n");
            exit(EXIT_FAILURE);
        }
    }

    ret = uart_close(fd);
    if(ret == -1)
    {
        fprintf(stderr,"uart_close error\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}