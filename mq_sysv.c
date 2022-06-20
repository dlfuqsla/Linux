#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define IPC_KEY_FILENAME  "/proc"					// IPC_KEY_FILENAME  이름으로 정의하고 내용은 /proc 이다.
#define IPC_KEY_PROJ_ID   'a'						// IPC_KEY_PROJ_ID 의 이름으로 정의하고 내용은 'a' 이다.
#define MSGBUF_STR_SIZE  64						// MSGBUF_STR_SIZE 를 정의하는데 내용은 64로 정의한다.


struct msgbuf {								// 구조체 msgbuf를 정의한다. 
	long mtype;							// long 타입의 mtype변수
	char string[MSGBUF_STR_SIZE];					// char 타입의 string 변수 크기는 사전에 정의한대로 64크기이다.
};



static void print_usage(const char *progname)				// print_usage 함수 정의. 파라미터로 받는값은 char타입을 받는다 받은 후 이름은 progname으로 지정
{
	printf("%s (send|recv) MTYPE\n", progname);			// progname을 출력하고 (send|recv) MTYPE 을출력한다. 한마디로 프로그램 사용법에 대해 출력함.
}

static int init_msgq(void)						// init_msgq 함수 정의
{
        int msgq;							// int msgq 변수 선언
        key_t key;							// key_t key 구조체 선언

        key = ftok(IPC_KEY_FILENAME, IPC_KEY_PROJ_ID);			// ftok 함수를 사용하여 반환 받은 값을 key에 넣는다. 이때 파라미터값은 파일의 이름, 파일의 프로젝트 ID이다.(사전에 정의 완료)
        if (key == -1){							// 만약 반환받은 key 값이 -1, 즉 실패하면 에러문 출력하고 -1리턴.
                perror("ftok()");
                return -1;
        }

        msgq = msgget(key, 0644 | IPC_CREAT);				// msqID를 리턴 받는 msgget()함수를 실행시킨다. 파라미터는 위에서 구한 key값, 퍼미션, Creat 할수있도록 플래그를 줌)
        if(msgq == -1) {						// 만약 리턴받은 값이 -1이면 에러문 출력
                perror("msgget()");
                return -1;

        }
	return msgq;							// msgq값을 넘겨준다.
}


static int do_send(long mtype)						// do_send 함수 정의. 파라미터로 받는 값은 long 타입의 mtype 변수
{
	int msgq;							// int msgq 선언
	struct msgbuf mbuf;						// 구조체 msgbuf를 mbuf 이름으로 선언 (함수 내에서 mbuf의 이름으로 사용할수있다)


	msgq = init_msgq();						// msgq 에 init_msgq()함수를 실행한 결과값을 리턴받는다.
	if (msgq == -1) {						// 만약 -1이 리턴되었다면 에러가 발생했다는 뜻이므로 에러문출력
		perror("init_msgq()");
		return -1;
	}
	
	memset(&mbuf, 0, sizeof(mbuf));					// memset()함수로 buf안의 내용을 초기화한다.
	mbuf.mtype = mtype;						// mbuf구조체 안의 mtype에 파라미터로 받은 mtype값을 넣는다.
	snprintf(mbuf.string, sizeof(mbuf.string),"hello world mtype %ld", mtype);	//snprintf()함수를 이용하여 mbuf구조체의 string에 string의 길이 만큼 hello world mtype을 넣어준다.

	if(msgsnd(msgq, &mbuf, sizeof(mbuf.string),0)== -1){		//msgsnd()함수를 이용하여 msgq(ID),mbuf에 저장되어있는 데이터를 string의 사이즈 만큼 넣어 보낸다 마지막 0은 recv로 받지 않아
										내부에 공간이 없을경우 생길때 가지 대기하도록 하는 설정값.
		perror("msgsnd()");					// 만약 -1이 리턴된다면 실패했다는 뜻으로 에러를 출력한다.
		return -1;
	}
	return 0;
}

static int do_recv(long mtype)						// do_recv 함수 정의부분 파라미터로 받는값은 long mtype이다.
{
	int msgq;      							// msgq_id를 받아야하므로 int msgq변수선언
        struct msgbuf mbuf;						// 구조체 msgbuf를 mbuf의 이름으로 사용하기위해 선언
	int ret;							// msgrcv()로 받는 값을 저장하기 위해 ret 변수 선언

        msgq = init_msgq();						// init_msgq()함수를 이용해 msgq_id를 리턴받기
        if (msgq == -1) {						// 실패하면 에러문 출력
                perror("init_msgq()");
                return -1;
        }

        memset(&mbuf, 0, sizeof(mbuf));					// 메시지를 받기전에 memset()을 이용하여 mbuf를 초기화
       	ret = msgrcv(msgq, &mbuf, sizeof(mbuf.string), mtype, 0); 	// ret 에 mbuf.string의 바이트 카운트를 저장한다.
									// 파라미터값은 msgq(ID), mbuf 구조체, string 사이즈 값 mtype(양수), 0을 입력받는다.
	if (ret == -1){							// 실패시 -1리턴.
		perror("msgrcv()");
		return -1;
	}
	printf("received msg: mtype %ld, msg [%s]\n",			// 출력문 사용 received msg: mtype , msg를 출력한다.
		mbuf.mtype, mbuf.string);				// mbuf에 저장된 타입, 문자열을 가져온다.

	return 0;
}


int main(int argc, char **argv)						// 메인 함수 정의 
{
	int ret;							// 사용할 변수 선언 int ret, long mtype
	long mtype;
	


	if(argc <3) {							// 3보다 작으면 사용법 출력
		print_usage(argv[0]);					// 3보다 작다는 것은 send, mtype을 지정해주기때문에 3보다는 커야한다.
		return -1;
	}	

	mtype = strtol(argv[2], NULL, 10);				// 문자열의 값을 long타입으로 변환시켜준다.  

	if( !strcmp(argv[1], "send")){					// 비교 함수 send 면 아래 문장 실행
		if(mtype <=0){						// mtype이 0보다 작으면 사용법 출력하고 종료
			print_usage(argv[0]);				// 즉 send 를 입력해야하고 동시에 mtype은 0보다 큰 양수를 입력받아야만 한다.
			return -1;		
		}
		ret = do_send(mtype);					// do_send 함수 실행 파라미터로 mtype을 던져줌
	} else if (!strcmp(argv[1], "recv")) {				// 위에 해당하지 않으면 recv 함수 실행
		ret = do_recv(mtype);	

	} else {
		print_usage(argv[0]);					// 다 해당하지 않을 시 사용법 출력
		return -1;
	}


	return ret;

}
