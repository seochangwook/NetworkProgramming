#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h> //DIR 포인터 변수랑 연동되는 구조체 헤더파일.//
#include<sys/types.h> //여러 타입을 정의.//
#include<fcntl.h> //파일디스크립터를 제어.//
#include<unistd.h> //여려가지 설정관련한 시스템 콜이 포함.//
#include<sys/stat.h> //파일, 디렉터리의 정보를 포함하는 시스템 콜이 포함.//
#include<time.h> //시간에 따른 시스템 콜.//
#include<signal.h> //시그널 관련 시스템 콜.//
#include<pthread.h> //스레드 관련 시스템 콜. -lpthread 링커옵션 필요.//
#include<errno.h> //에러처리에 관한 시스템 콜.//
#include<pwd.h> // /etc/passwd관련한 시스템 콜이 들어있는 헤더파일.//
#include<grp.h> // /etc/group관련 시스템 콜이 들어있는 헤더파일.//
#include<math.h> //수학연산 라이브러리 헤더파일 -lm을 가지고 컴파일 필요.//
#include<sys/time.h> //시간관련 시스템 콜 관련 라이브러리.//
#include<sys/wait.h>
#include<netinet/in.h>
#include<sys/socket.h> //소켓관련 라이브러리 함수.//
#include<netdb.h> //포트,서비스 정보관련 라이브러리 함수.//
#include<arpa/inet.h> //IP주소에 대해서 변환함수를 가지고 있는 라이브러리.//
#include<netinet/ip.h> //RAW소켓 사용 시 ip의 헤더정보를 위한 라이브러리.//
#include<netinet/tcp.h> //RAW소켓 사용 시 tcp의 헤더정보를 위한 라이브러리.//
#include<netinet/ip_icmp.h> //RAW소켓 사용 시 ICMP의 헤더정보를 위한 라이브러리.//
#include<netinet/in_systm.h>
#include<sys/resource.h> //getrlimit시스템 콜 사용.//
#include<sys/ioctl.h>
#include<net/if.h>
/////////////////////////
//#define부분.//
#define TCP_SET 6 //TCP값.//
#define UDP_SET 17
#define ACCEPT_OPTION 1
#define DROP_OPTION 0
#define ACCEPT_DROP_NONE -1 //-1이면 아무것도 ACCEPT,DROP에 대해서 아무것도 설정이 안되있다는 것.//
#define ERROR_IP_EXCEPTION 256 //최대 초과 IP의 범위.//
#define ERROR_PORT_EXCEPTION 65536 //최대 초과 포트 범위.//
#define IP_MIN_ALL_OPTION "0.0.0.0" //최소 IP의 디폴트 옵션.//
#define IP_MAX_ALL_OPTION "255.255.255.255" //최대 IP의 디폴트 옵션.//
#define PORT_ALL_OPTION 0 //최소 포트 값.//
#define CONTROLBIT_NOT_OPTION 0 //디폴트는 다 컨트롤 비트를 0으로 초기화.//
#define MAX_RULL 100 //최대 룰을 정의.//
#define CONTROL_BIT_SIZE 6 //컨트롤 비트.//
//////////////////////////
//rull 구조체 부분.//
typedef struct rull_set
{
	int protocol;
	char *saddr_s, *saddr_e; //시작 IP주소(범위)//
	int s_ip_accept_drop_check; //전송ip수준에서 access, drop부분.//
	char *daddr_s, *daddr_e; //종료 IP주소(범위)//
	int d_ip_accept_drop_check; //IP수준에서 ACCEPT/DROP
	int source_port; //시작 포트번호.//
	int source_port_accept_drop_check; //source port수준에서 ACCEPT/DROP
	int dest_port; //도착 포트번호.//
	int dest_port_accept_drop_check; //dest port수준에서 ACCEPT/DROP
	int set_controlbit_array[6]; //6개의 경우로 ControlBit를 고정.//
	int set_controlbit_accept_drop_check; //Controlbit수준에서의 ACCEPT/DROP
}rull_set;
/////////////////////////
//함수부분.//
void Print_Menu(); //메뉴를 출력.//
void Print_Packet_info(struct iphdr *iph, struct tcphdr *tcph);//비옵션일때 패킷정보 출력.-> 로그파일에만 저장하고 다른 작업은 없다.//
void Check_Packet_info(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull); //룰을 체크하는 부분.//
void Read_Packet_Not_Rull(int raw_socket); //비규칙 모드에서의 패킷읽기.//
void Read_Packet_Rull(int raw_socket, struct rull_set *rull); //비규칙 모드에서의 패킷읽기.//
int Rull_Mode_Check(); //비규칙 모드, 규칙모드를 선택.//
void sig_handler(int sig_num); //시그널 처리 함수.//
void Rull_set(struct rull_set *rull); //룰을 셋팅하는 함수.//
void Rull_initialize(struct rull_set *rull); //규칙을 초기화 하는 함수.//
void Rull_view(struct rull_set *rull); //룰 정보보기.//
void Rull_register(); //룰을 등록하는 부분. 파일이 전역변수라 따로 인자가 필요없음.//
void Rull_remove(); //룰을 제거.//
void Rull_file_Print(); //룰을 열람하는 함수. 파일이 전역변수라 따로 인자가 필요없음.//
int Rull_Check_protocol(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull); //룰에 따라서 protocol부분 검사.//
int Rull_Check_source_ip(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull); //룰에 따라서 Source ip부분에 대한 검사.//
int Rull_Check_dest_ip(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull); //룰에 따라서 Destination ip부분에 대한 검사.//
int Rull_Check_Source_port(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull); //룰에 따라서 Source port부분에 대한 검사.//
int Rull_Check_Dest_port(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull); //룰에 따라서 Dest port부분에 대한 검사.//
int Rull_Check_ControlBit(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull); //룰에 따라서 ControlBit 검사.//
void save_total_log_file(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull); //전체 로그파일에 저장하는 함수.//
void save_accept_log_file(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull); //ACCEPT의 대상의 패킷들이 저장되는 로그함수.//
void save_drop_log_file(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull, char error_message[]); //drop되는 패킷들이 저장되는 로그함수(이유가 필요)//
//////////////////////////
//전역변수 부분.//
//로그파일 부분.//
FILE *total_packet_log_file = NULL; //전체 필터링하는 패킷을 저장하는 파일.//
FILE *correct_packet_log_file = NULL; //허용한 패킷의 정보가 저장되는 파일.//
FILE *incorrect_packet_log_file = NULL; //허용하지 않은 패킷의 정보가 저장되는 파일.//

//옵션설정 정보 파일.//
FILE *ip_rull_set_file = NULL; //ip룰 설정파일.//
FILE *port_rull_set_file = NULL; //port룰 설정파일.//
FILE *controlbit_rull_set_file = NULL; //ControlBit룰 설정파일.//

struct tm *t; //시간관련 구조체.//
time_t timer; //시간측정.//

int rull_set_flag = 0; //0이면 룰이 셋팅되지 않음. 초기 방화벽 프로그램 실행 시 설정.//
int rull_check_count = 0; //룰에 대한 선택변수가 저장.//
char *multi_error_message = NULL; //다중 에러문.//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct pseudohdr{
	unsigned long s_addr;
	unsigned long d_addr;
	char zero;
	unsigned char protocol;
	unsigned short length;
};

pthread_t thread;
#define LOCAL_IP "127.0.0.1"
#define LOCAL_PORT 0
int play_start = 0;
void *do_check(void * arg);
void sendtodata(int sd);
unsigned short cksum_in(unsigned short *addr, int len);
void log_view(void);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////
int main(int argc, char *argv[], char *envp[])
{
	int user_select; //메뉴선택 관련 변수.//
	int raw_socket; //로우소켓.//
	int mode_check; //모드 선택변수.//
	int Rull_initialize_Anser = 0;

	struct sigaction act; //시그널 처리 구조체.//
	struct rull_set rull; //방화벽 관련 룰 구조체.//

	//시그널 등록.//
	act.sa_handler = sig_handler;
	act.sa_flags = SA_RESTART;
	sigfillset(&act.sa_mask);

///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////


	sigaction(SIGINT, &act, NULL); //시그널을 대입. 시그널의 사용은 패킷 필터링을 중지하기 위해서이다.//

	while(1)
	{
		Print_Menu(); //메뉴 출력.//

		printf("\033[%dm\n* Select Menu Number : \033[0m", 32);
		scanf("%d", &user_select);

		if(user_select == 8)
		{
			printf("Program Exit...\n");
			
			break;
		}

		//사용자의 선택 구조는 switch/case로 이루어진다.//
		switch(user_select)
		{
		case 1: //패킷추적 시작.//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			

			mode_check = Rull_Mode_Check(); //1이면 규칙모드, 0이면 비규칙 모드. -1이면 뒤로 가기//
			//뒤로 가기 기능
			if(mode_check == -1){
				system("clear");
			}
			else if(mode_check == 0 || mode_check == 1)
			{
				play_start = 1;

				//IPV4그룹과 RAW타입, TCP프로토콜로 이루어진 소켓을 만든다.분석하고자 하는 IP의 데이터는 TCP이다.//
				if((raw_socket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP)) < 0)
				{
					printf("socket open error\n");
	
					exit(-1);
				}

				if(play_start == 1){
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

					if(mode_check == 1) //규칙모드.//
					{
						if(rull_set_flag == 0)
						{
							printf("\n\033[%dmThis rule has not been set. First, please set...\033[0m\n", 31);
						}

						else if(rull_set_flag == 1)
						{
							sleep(1);
							system("clear");

							printf("\n");
					
							printf("\033[%dm* Please select the Combination you want to apply... \033[0m\n", 32);
							printf("\n* Source IP : SI / Destination IP : DI / Source Port : SP / Destination Port : DP / ControlBit : CB\n"); 
							printf("* Users can choose the level of the firewall.\n");	
							printf("\033[%dm* The shielding will step up increasingly stronger.\n\033[0m", 31);

							printf("\n\033[%dm     < Level 1 >\033[0m    |\033[%dm      < Level 2 >  \033[0m     |\033[%dm      < Level 3 >   \033[0m   |\033[%dm       < Level 4 >  \033[0m     |\033[%dm       < Level 5 >   \033[0m   |", 37, 33, 36, 35, 31);
							printf("\n");
							printf("        1) SI       |       6) SI+DI         |     16) SI+DI+SP      |     26) SI+DI+SP+DP     |   31) SI+DI+SP+DP+CB   |\n");
							printf("        2) DI       |       7) SI+SP         |     17) SI+DI+DP      |     27) SI+DI+SP+CB     |                        |\n");
							printf("        3) SP       |       8) SI+DP         |     18) SI+DI+CB      |     28) SI+DI+DP+CB     |                        |\n");
							printf("        4) DP       |       9) SI+CB         |     19) SI+SP+DP      |     29) DI+SP+DP+CB     |                        |\n");
							printf("        5) CB       |      10) DI+SP         |     20) SI+SP+CB      |     30) SI+SP+DP+CB     |                        |\n");
							printf("                    |      11) DI+DP         |     21) SI+DP+CB      |                         |                        |\n");
							printf("                    |      12) DI+CB         |     22) DI+SP+DP      |                         |                        |\n");
							printf("                    |      13) SP+DP         |     23) DI+DP+CB      |                         |                        |\n");
							printf("                    |      14) SP+CB         |     24) SP+DP+CB      |                         |                        |\n");
							printf("                    |      15) DP+CB         |     25) DI+SP+CB      |                         |                        |\n");

							printf("\n\n\033[%dmSelect Firewall Level(0 is not Rull) -> \033[0m", 33);
							scanf("%d", &rull_check_count);
		
							system("clear");

							Rull_view(&rull); //룰 정보 보기.//	

							printf("\n* [%d number] Rull apply...\n", rull_check_count);
					
							if(rull_check_count == 1 || rull_check_count == 2 || rull_check_count == 3 || rull_check_count == 4 || rull_check_count == 5)
							{
								printf("\033[%dm* It is being applied to the amount of Level 1.\n\033[0m", 31);
							}

							else if(rull_check_count == 6 || rull_check_count == 7 || rull_check_count == 8 || rull_check_count == 9 || rull_check_count == 10 ||
								 rull_check_count == 11 || rull_check_count == 12 || rull_check_count == 13 || rull_check_count == 14 || rull_check_count == 
								 15)
							{
								printf("\033[%dm* It is being applied to the amount of Level 2.\n\033[0m", 31);
							}	

							else if(rull_check_count == 16 || rull_check_count == 17 || rull_check_count == 18 || rull_check_count == 19 ||
								 rull_check_count == 20 || rull_check_count == 21 || rull_check_count == 22 || rull_check_count == 23 ||
								 rull_check_count == 24 || rull_check_count == 25)
							{
								printf("\033[%dm* It is being applied to the amount of Level 3.\n\033[0m", 31);
							}

							else if(rull_check_count  == 26 || rull_check_count  == 27 || rull_check_count  == 28 || rull_check_count  == 29 ||
								 rull_check_count  == 30)
							{
								printf("\033[%dm* It is being applied to the amount of Level 4.\n\033[0m", 31);
							}

							else if(rull_check_count == 31)
							{
								printf("\033[%dm* It is being applied to the amount of Level 5.\n\033[0m", 31);
							}

							printf("--------------------------------------------------\n");

							pthread_create(&thread, NULL, do_check, (void *) raw_socket);////////////////////////////////////////////////////////////////////
							Read_Packet_Rull(raw_socket, &rull); //룰을 보여주어야 하니 구조체 변수를 넘김.//
						}
					}

					else if(mode_check == 0) //비규칙 모드.//
					{
						pthread_create(&thread, NULL, do_check, (void *) raw_socket);///////////////////////////////////////////////////////////////////
						Read_Packet_Not_Rull(raw_socket); //비규칙이니 소켓만 넘김.//
					}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				}
			}
			close(raw_socket); //소켓을 닫는다.//
			break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		case 2: //옵션 정보보기.(메뉴얼 정보)//	
			printf("--------------------------------\n");

			printf("Information About Options\n\n");
			
			Rull_file_Print(); //파일에 저장된 룰을 보여주는 함수. 설명포함.//

			printf("--------------------------------\n");

			break;

		case 3: //옵션정보 등록.//
			printf("--------------------------------\n");

			printf("Registration options\n\n");

			Rull_register(); //룰을 등록.//

			printf("--------------------------------\n");

			break;

		case 4: //옵션제거 부분.//
			printf("--------------------------------\n");

			printf("Removal options\n\n");
			
			Rull_remove(); //룰 제거.//

			printf("--------------------------------\n");

			break;

		case 5: //옵션설정.//
			printf("--------------------------------\n");

			printf("Options set in progress ...\n\n");

			printf("Are you sure you want to initialize the rules?(yes = 1, No = -1)\n\n");

			scanf("%d",&Rull_initialize_Anser);

			if(Rull_initialize_Anser == 1)
			{
				//총 4초의 시간이 걸린다.(초기화 2초 + 설정 2초)//
				Rull_initialize(&rull); //룰을 초기화.//

				Rull_set(&rull);//룰 설정 작업.//
			
				rull_set_flag = 1; //옵션이 한 번 이상 설정될 시 1로 변경.//
			}
	
			printf("--------------------------------\n");

			break;

		case 6: //현재 설정된 옵션정보 보기.//
			printf("--------------------------------\n");

			printf("Option set information\n\n");

			if(rull_set_flag == 0)
			{
				printf("\033[%dmThis rule has not been set. First, please set...\033[0m\n", 31);
			}

			else if(rull_set_flag == 1)
			{
				Rull_view(&rull); //룰 정보 보기.//
			}

			printf("--------------------------------\n");

			break;

		case 7: //현재까지의 로그정보 보기.//
			printf("--------------------------------\n");

			printf("View log information\n\n");

			log_view();///////////////////////////////////////////////////////////////////////////////////////////

			printf("--------------------------------\n");

			break;
		}
	}

	close(raw_socket); //소켓을 닫는다.//

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void log_view(void)
{

	FILE      *fp;
	char       ch;
	int file_select = 0;

	printf(" 1.ACCEPT_PACKET.txt\n 2.DROP_PACKET.txt\n 3.TOTAL_PACKET.txt\n 4.Back to the Menu\n then choose the number of the single select : ");
	scanf("%d", &file_select);

	if(file_select == 1){
		printf("\033[%dm\n*************** 1. ACCEPT_PACKET.txt : \033[0m", 33);
		printf("\033[%dm************************************************* \033[0m\n", 33);
		if ( fp = fopen( "ACCEPT_PACKET.txt", "r"))
		{
			while( EOF != (ch = fgetc(fp)))
			{
				putchar(ch);
			}
			fclose(fp);
		}
		printf("\033[%dm\n************************************************************************************** \033[0m\n", 33);

	} else if(file_select == 2){
		printf("\033[%dm\n*************** 2. DROP_PACKET.txt : \033[0m", 33);
		printf("\033[%dm**************************************************** \033[0m\n", 33);
		if ( fp   = fopen( "DROP_PACKET.txt", "r"))
		{
			while( EOF != (ch = fgetc(fp)))
			{
				putchar( ch);
			}
			fclose( fp);
		}
		printf("\033[%dm\n************************************************************************************** \033[0m\n", 33);

	} else if(file_select == 3){
		printf("\033[%dm\n*************** 3.TOTAL_PACKET.txt : \033[0m", 33);
		printf("\033[%dm******************************************************* \033[0m\n", 33);
		if ( fp   = fopen( "TOTAL_PACKET.txt", "r"))
		{
			while( EOF != (ch = fgetc(fp)))
			{
				putchar( ch);
			}
			fclose( fp);
		}
		printf("\033[%dm\n************************************************************************************** \033[0m\n", 33);
	}
}



void *do_check(void *arg)
{
	int raw_socket = (int)arg;
	char end[ ] = "end\n";
	char endData[1000];
	int n = 0; 
	while(1)
	{
		int n = 0;
		endData[0] = '\0';
		if((n = read(0, endData, sizeof(endData))) > 0)
		{
			if(strncmp(endData, end, strlen(end)) == 0) {
				play_start = 0;
				sendtodata(raw_socket);
				break;
			}
			
		}
	}
}
void sendtodata(int sd)
{
	int port = 0;

	int on =1;

	int len;

	int tx_packet_size = sizeof(struct iphdr) + sizeof(struct tcphdr) + sizeof(struct pseudohdr);

	int rx_packet_size = sizeof(struct iphdr) + sizeof(struct tcphdr);
	char *rx_packet = (char *)malloc(rx_packet_size);
	char *tx_packet = (char *)malloc(tx_packet_size);

	struct tcphdr *tcph,*rx_tcph;
	struct iphdr *iph,*rx_iph;
	struct pseudohdr *pseudoh;

	struct in_addr s_addr,d_addr;
	struct sockaddr_in local,remote;
	struct servent *serv;

	iph = (struct iphdr *)(tx_packet);
	tcph = (struct tcphdr *)(tx_packet +sizeof(struct iphdr));
	pseudoh = (struct pseudohdr *)(tx_packet + sizeof(struct iphdr)+sizeof(struct tcphdr));

	if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0) {
		printf("set socket option error\n"); exit(-2);
	}

	memset(tx_packet,0,tx_packet_size);

	//d_addr.s_addr=target;
	d_addr.s_addr=inet_addr(LOCAL_IP);
	s_addr.s_addr=inet_addr(LOCAL_IP);

	pseudoh->s_addr = s_addr.s_addr;
	pseudoh->d_addr = d_addr.s_addr;
	pseudoh->protocol = IPPROTO_TCP;
	pseudoh->zero = 0;
	pseudoh->length = htons(sizeof(struct tcphdr));

	tcph->source = htons(LOCAL_PORT);
	tcph->dest= htons(port);
	tcph->seq= htons(random()%time(NULL));
	tcph->ack_seq= 0;
	tcph->doff= 5;
	tcph->res1= 0;
	tcph->fin= 0;
	tcph->syn= 1;
	tcph->rst= 0;
	tcph->psh= 0;
	tcph->ack= 0;
	tcph->urg= 0;
	tcph->window= htons(1024);
	tcph->check= (unsigned short)cksum_in((unsigned short *)tcph,(sizeof(struct tcphdr) + sizeof(struct pseudohdr)));

	iph -> ihl = 5;
	iph -> version = 4;
	iph -> tos = 0;
	iph -> tot_len = htons(tx_packet_size) - sizeof(struct pseudohdr);
	iph -> id = 0;
	iph -> frag_off = 0;
	iph -> ttl = IPDEFTTL;
	iph -> protocol = IPPROTO_TCP;
	iph -> saddr = s_addr.s_addr;
	iph -> daddr = d_addr.s_addr;
	iph -> check = (unsigned short)cksum_in((unsigned short *)iph, sizeof(struct iphdr));
	
	remote.sin_family = PF_INET;
	remote.sin_addr = d_addr;
	remote.sin_port = htons(port);
	remote.sin_port = 0;

	if(sendto(sd, tx_packet, (tx_packet_size - sizeof(struct pseudohdr)), 0x0, (struct sockaddr *)&remote, sizeof(remote)) < 0) {
		printf("send error \n"); exit(-3);
	}
}
unsigned short cksum_in(unsigned short *addr, int len)
{
	unsigned long sum = 0;
	unsigned short answer = 0;
	unsigned short *w = addr;
	int nleft = len;

	while(nleft > 1) {
		sum += *w++;
		if(sum & 0x80000000)
			sum = (sum & 0xffff) + (sum >> 16);
			nleft -= 2;
	}

	if(nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	while(sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16); 

	return(sum == 0xffff)?sum:~sum;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////
void Print_Menu() //메뉴 출력.//
{
	printf("\n");
	printf("<< Firewall Program >>\n");
	printf("1. Starting packet detection\n");
	printf("2. Information About Options(Rule)\n");
	printf("3. Registration options(Rule)\n");
	printf("4. Removal options(Rule)\n");
	printf("5. Option(Rule) Settings\n");
	printf("6. Option(Rule) View\n");
	printf("7. View log information\n");
	printf("8. Exit\n");
}
///////////////////////
int Rull_Mode_Check()
{
	int rull_select;

	printf("\033[%dm* Select Mode( Rull -> 1 / Not Rull -> 0/ Back to the Menu -> -1) : \033[0m", 33);
	scanf("%d", &rull_select);
	
	return rull_select;
}
///////////////////////
void Rull_file_Print()
{
	//IP부분 변수.//
	int number;
	int port_number;

	char str_1[30];
	char str_2[30];
	char str_3[30];
	char str_4[30];
	char str_5[30];
	char str_6[30];
	char str_7[30];
	char str_8[30];
	char str_9[30];
	char str_10[30];
	char str_11[30];
	char str_12[30];
	char str_13[30];

	printf("\033[%dm<< View the entire rule >>\n\n\033[0m", 33);
	
	//전체 정보를 다 보여주어야 하기에 파일 3개를 오픈.//
	if((ip_rull_set_file = fopen("ip_rull", "r")) == NULL)
	{
		fprintf(stderr, "ip rull file open error\n");
		exit(1);
	}

	if((port_rull_set_file = fopen("port_rull", "r")) == NULL)
	{
		fprintf(stderr, "port rull file open error\n");
		exit(1);
	}

	if((controlbit_rull_set_file = fopen("cb_rull", "r")) == NULL)
	{
		fprintf(stderr, "controlbit file open error\n");
		exit(1);
	}

	//IP부분의 룰을 출력.//
	printf("\033[%dm * IP Rull *\033[0m\n", 31);

	while(!feof(ip_rull_set_file))
	{
		fscanf(ip_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, 			str_10);

		printf("%d %s %s %s %s %s %s %s %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, str_10);
	}

	fclose(ip_rull_set_file);

	printf("\n\033[%dm* PORT Rull *\n\033[0m", 31);

	while(!feof(port_rull_set_file))
	{
		fscanf(port_rull_set_file, "%d %s %s %s %s %s %d %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, &port_number,str_6, str_7, str_8);

		printf("%d %s %s %s %s %s %d %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5,port_number, 
		str_6, str_7, str_8);
	}

	fclose(port_rull_set_file);

	printf("\n\033[%dm* CB Rull *\n\033[0m", 31);

	while(!feof(controlbit_rull_set_file))
	{
		fscanf(controlbit_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, str_10, str_11, str_12);

		printf("%d %s %s %s %s %s %s %s %s %s %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, str_10, str_11, str_12);
	}

	fclose(controlbit_rull_set_file);
}
////////////////////////
void Rull_register()
{
	int ip_port_cb_check;
	int final_number;

	//IP부분 변수.//
	int number;
	int port_number;

	//출력할 데이터 값을 받기위한 문자열 변수.//
	char str_1[30];
	char str_2[30];
	char str_3[30];
	char str_4[30];
	char str_5[30];
	char str_6[30];
	char str_7[30];
	char str_8[30];
	char str_9[30];
	char str_10[30];
	char str_11[30];
	char str_12[30];

	printf("\033[%dm<< Rule registration section >>\033[0m\n", 33);

	printf("Choose your ip, port, Control Bit to register (\033[%dm0 -> ip / 1 -> port / 2 -> Controlbit/ -1 -> Back to the Menu\033[0m) : ", 31);
	scanf("%d", &ip_port_cb_check);

	if(ip_port_cb_check == 0)
	{
		printf("\n* Input Format the IP(Sample) :\n");
		printf("< number iptables -OPTION INPUT IP s_s/s_e [Start_ip_address] ~ [End_ip_address] DROP [USER] >\n"); 
		printf("< number iptables -OPTION OUTPUT IP d_s/d_e [Start_ip_address] ~ [End_ip_address] DROP [USER] >\n\n"); 

		if((ip_rull_set_file = fopen("ip_rull", "r")) == NULL)
		{
			fprintf(stderr, "ip rull file open error\n");
			exit(1);
		}

		//등록할 버튼의 정보를 불러온다.//
		while(!feof(ip_rull_set_file))
		{
			fscanf(ip_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, 				str_10);

			final_number = number;
		}

		if((ip_rull_set_file = fopen("ip_rull", "a")) == NULL)
		{
			fprintf(stderr, "ip rull file open error\n");
			exit(1);
		}
		
		//마지막 부분의 번호를 불러온다.//
		final_number++;
	
		printf("* Enroll Input number : %d\n", final_number);

		printf("* Input -> \n");

		char rull_msg[BUFSIZ]; //룰 정보보기.//

		int recv_size = read(0, rull_msg, BUFSIZ); //터미널로 부터 파일에 넣을 데이터 값을 입력.//

		if(recv_size == -1)
		{
			fprintf(stderr, "read() error\n");
			exit(1);
		}

		rull_msg[recv_size] = '\0'; //문자열 이기에 마지막엔 NULL을 넣는다.//

		printf("%s\n", rull_msg);
		printf("recv size : %d\n", recv_size);

		//파일에 현재 정보를 저장.//
		fprintf(ip_rull_set_file, "%s", rull_msg); 

		fclose(ip_rull_set_file); //파일을 닫는다.//
		memset(&rull_msg, 0, sizeof(char)*BUFSIZ);
	}

	else if(ip_port_cb_check == 1)
	{
		printf("* Input Format the PORT :\n");
		printf("< number iptables -OPTION OUTPUT SOURCE_PORT sport [sport_number] state DROP/ACCEPT [USER] >\n"); 
		printf("< number iptables -OPTION INPUT DEST_PORT dport [dport_number] state DROP/ACCEPT [USER] >\n\n"); 

		if((port_rull_set_file = fopen("port_rull", "r")) == NULL)
		{
			fprintf(stderr, "port rull file open error\n");
			exit(1);
		}

		//등록할 버튼의 정보를 불러온다.//
		while(!feof(port_rull_set_file))
		{
			fscanf(port_rull_set_file, "%d %s %s %s %s %s %d %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, &port_number, 			str_6, str_7, str_8);

			final_number = number;
		}

		if((port_rull_set_file = fopen("port_rull", "a")) == NULL)
		{
			fprintf(stderr, "port rull file open error\n");
			exit(1);
		}
		
		//마지막 부분의 번호를 불러온다.//
		final_number++;
	
		printf("* Enroll Input number : %d\n", final_number);

		printf("* Input -> \n");

		char rull_msg[BUFSIZ]; //룰 정보보기.//

		int recv_size = read(0, rull_msg, BUFSIZ); //터미널로 부터 파일에 넣을 데이터 값을 입력.//

		if(recv_size == -1)
		{
			fprintf(stderr, "read() error\n");
			exit(1);
		}

		rull_msg[recv_size] = '\0'; //문자열 이기에 마지막엔 NULL을 넣는다.//

		printf("%s\n", rull_msg);
		printf("recv size : %d\n", recv_size);

		//파일에 현재 정보를 저장.//
		fprintf(port_rull_set_file, "%s", rull_msg); 

		fclose(port_rull_set_file); //파일을 닫는다.//
		memset(&rull_msg, 0, sizeof(char)*BUFSIZ);
	}

	else if(ip_port_cb_check == 2)
	{
		printf("* Input Format the ControlBit :\n");
		printf("< number iptables -OPTION INPUT/OUTPUT TCP_FLAG [URG ACK PSH RST SYN FIN] DROP/ACCEPT [USER] >\n\n"); 

		if((controlbit_rull_set_file = fopen("cb_rull", "r")) == NULL)
		{
			fprintf(stderr, "cb rull file open error\n");
			exit(1);
		}

		//등록할 버튼의 정보를 불러온다.//
		while(!feof(controlbit_rull_set_file))
		{
			fscanf(controlbit_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, 				str_8, str_9, str_10, str_11, str_12);

			final_number = number;
		}

		if((controlbit_rull_set_file = fopen("cb_rull", "a")) == NULL)
		{
			fprintf(stderr, "port rull file open error\n");
			exit(1);
		}
		
		//마지막 부분의 번호를 불러온다.//
		final_number++;
	
		printf("* Enroll Input number : %d\n", final_number);

		printf("* Input -> \n");

		char rull_msg[BUFSIZ]; //룰 정보보기.//

		int recv_size = read(0, rull_msg, BUFSIZ); //터미널로 부터 파일에 넣을 데이터 값을 입력.//

		if(recv_size == -1)
		{
			fprintf(stderr, "read() error\n");
			exit(1);
		}

		rull_msg[recv_size] = '\0'; //문자열 이기에 마지막엔 NULL을 넣는다.//

		printf("%s\n", rull_msg);
		printf("recv size : %d\n", recv_size);

		//파일에 현재 정보를 저장.//
		fprintf(controlbit_rull_set_file, "%s", rull_msg); 

		fclose(controlbit_rull_set_file); //파일을 닫는다.//
		memset(&rull_msg, 0, sizeof(char)*BUFSIZ);
	}
}
////////////////////////
void Rull_remove()
{
	char buffer[1024]; 
	char rull_buf[MAX_RULL][1024]; //문자열 배열.//
	int remove_option_number; //지울 옵션의 번호.//
	int count = 0;
	int rull_count = 0;
	int option_check;
	int i;

	//IP부분 변수.//
	int number;
	int port_number;

	//출력할 데이터 값을 받기위한 문자열 변수.//
	char str_1[30];
	char str_2[30];
	char str_3[30];
	char str_4[30];
	char str_5[30];
	char str_6[30];
	char str_7[30];
	char str_8[30];
	char str_9[30];
	char str_10[30];
	char str_11[30];
	char str_12[30];

	printf("Please enter the options to remove (\033[%dmIP -> 1 / PORT -> 2 / Control Bit -> 3 / Back to the Menu -> -1\033[0m) : ", 31);
	scanf("%d", &option_check);  

	if(option_check == 1)
	{	
		if((ip_rull_set_file = fopen("ip_rull", "r")) == NULL)
		{
			fprintf(stderr, "ip rull file open error\n");
			exit(1);
		}	

		printf("\n<Currently registered options (rule) List>\n\n");

		while(!feof(ip_rull_set_file))
		{
			fscanf(ip_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, 				str_10);

			printf("%d %s %s %s %s %s %s %s %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, str_10);
			rull_count++;
		}

		fclose(ip_rull_set_file);

		if((ip_rull_set_file = fopen("ip_rull", "r")) == NULL)
		{
			fprintf(stderr, "ip rull file open error\n");

			exit(1);
		}	

		printf("\033[%dm\nInput remove option Number : \033[0m", 31);
		scanf("%d", &remove_option_number);

		while(fgets(buffer, 1024, ip_rull_set_file) != NULL) //fgets로 1024바이트씩 읽어온다.//
		{ 
			if(strchr(buffer, '\n') != NULL) //줄바꿈까지 한 줄로 인식해서 읽는다.//
			{
				count++; //구분을 짓기 위해 count증가.//
			}

		       if(remove_option_number == count) //지울 번호, 룰 부분에 도착할 시.//
			{ 
		      
		   	}

			else //지울려는 데이터 부분이 아닐경우.//
			{
				strcpy(rull_buf[count], buffer); //지울려는 숫자를 빼고 버퍼에 저장.//
			}
	      	} 

		fclose(ip_rull_set_file);

		if((ip_rull_set_file = fopen("ip_rull", "w")) == NULL)
		{
			fprintf(stderr, "ip_rull file open error\n");

			exit(1);
		}

		for(i=0; i<rull_count-1; i++)
		{
			fprintf(ip_rull_set_file, "%s", rull_buf[i]);
		}	

		fclose(ip_rull_set_file);
	}

	if(option_check == 2)
	{	
		if((port_rull_set_file = fopen("port_rull", "r")) == NULL)
		{
			fprintf(stderr, "port rull file open error\n");

			exit(1);
		}	

		printf("\n<Currently registered options (rule) List>\n\n");

		while(!feof(port_rull_set_file))
		{
			fscanf(port_rull_set_file, "%d %s %s %s %s %s %d %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, &port_number, 			str_6, str_7, str_8);

			printf("%d %s %s %s %s %s %d %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5, port_number, 
			str_6, str_7, str_8);
		}

		fclose(port_rull_set_file);

		if((port_rull_set_file = fopen("port_rull", "r")) == NULL)
		{
			fprintf(stderr, "port rull file open error\n");

			exit(1);
		}	

		printf("\033[%dm\nInput remove option Number : \033[0m", 31);
		scanf("%d", &remove_option_number);

		while(fgets(buffer, 1024, port_rull_set_file) != NULL) //fgets로 1024바이트씩 읽어온다.//
		{ 
			if(strchr(buffer, '\n') != NULL) //줄바꿈까지 한 줄로 인식해서 읽는다.//
			{
				count++; //구분을 짓기 위해 count증가.//
			}

		       if(remove_option_number == count) //지울 번호, 룰 부분에 도착할 시.//
			{ 
		      
		   	}

			else //지울려는 데이터 부분이 아닐경우.//
			{
				strcpy(rull_buf[count], buffer); //지울려는 숫자를 빼고 버퍼에 저장.//
			}
	      	} 

		fclose(port_rull_set_file);

		if((port_rull_set_file = fopen("port_rull", "w")) == NULL)
		{
			fprintf(stderr, "ip_rull file open error\n");

			exit(1);
		}

		for(i=0; i<count; i++)
		{
			fprintf(port_rull_set_file , "%s", rull_buf[i]);
		}	

		fclose(port_rull_set_file);
	}

	if(option_check == 3)
	{	
		if((controlbit_rull_set_file = fopen("cb_rull", "r")) == NULL)
		{
			fprintf(stderr, "cb rull file open error\n");

			exit(1);
		}	

		printf("\n<Currently registered options (rule) List>\n\n");

		while(!feof(controlbit_rull_set_file))
		{
			fscanf(controlbit_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, 				str_8, str_9, str_10, str_11, str_12);

			printf("%d %s %s %s %s %s %s %s %s %s %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, str_10, 				str_11, str_12);
		}

		fclose(controlbit_rull_set_file);

		if((controlbit_rull_set_file = fopen("cb_rull", "r")) == NULL)
		{
			fprintf(stderr, "port rull file open error\n");

			exit(1);
		}	

		printf("\033[%dm\nInput remove option Number : \033[0m", 31);
		scanf("%d", &remove_option_number);

		while(fgets(buffer, 1024, controlbit_rull_set_file) != NULL) //fgets로 1024바이트씩 읽어온다.//
		{ 
			if(strchr(buffer, '\n') != NULL) //줄바꿈까지 한 줄로 인식해서 읽는다.//
			{
				count++; //구분을 짓기 위해 count증가.//
			}

		       if(remove_option_number == count) //지울 번호, 룰 부분에 도착할 시.//
			{ 
		      
		   	}

			else //지울려는 데이터 부분이 아닐경우.//
			{
				strcpy(rull_buf[count], buffer); //지울려는 숫자를 빼고 버퍼에 저장.//
			}
	      	} 

		fclose(controlbit_rull_set_file);

		if((controlbit_rull_set_file = fopen("cb_rull", "w")) == NULL)
		{
			fprintf(stderr, "ip_rull file open error\n");

			exit(1);
		}

		for(i=0; i<count; i++)
		{
			fprintf(controlbit_rull_set_file , "%s", rull_buf[i]);
		}	

		fclose(controlbit_rull_set_file);
	}
}
///////////////////////
void Rull_view(struct rull_set *rull)
{
	sleep(1); //1초에 시간을 줌.//

	printf("<<Firewall Rull Info>>\n");

	int controlbit_count = 0;

	printf("* protocol : %d\n", rull->protocol);
	printf("* saddr_s : %s\n", rull->saddr_s);
	printf("* saddr_e : %s\n", rull->saddr_e);
	printf("* s_ip_accept_drop_check : %d\n", rull->s_ip_accept_drop_check);
	printf("* daddr_s : %s\n", rull->daddr_s);
	printf("* daddr_e : %s\n", rull->daddr_e);
	printf("* d_ip_accept_drop_check : %d\n", rull->d_ip_accept_drop_check);
	printf("* source_port : %d\n", rull->source_port);
	printf("* source_port_accept_drop_check : %d\n", rull->source_port_accept_drop_check);
	printf("* dest_port : %d\n", rull->dest_port);
	printf("* dest_port_accept_drop_check : %d\n", rull->dest_port_accept_drop_check);
	printf("* ControlBit(URG/ACK/PSH/RST/SYN/FIN) : ");

	for(controlbit_count = 0; controlbit_count < 6; controlbit_count++)
	{
		printf("%d ", rull->set_controlbit_array[controlbit_count]);
	}

	printf("\n");

	printf("* controlbit_accept_drop_check : %d\n", rull->set_controlbit_accept_drop_check);
}
///////////////////////
void Rull_initialize(struct rull_set *rull)
{
	//룰 구조체 설정.//
	int controlbit_count = 0;

	//시작주소, 목적지주소 설정.//
	rull->saddr_s = (char *)malloc(sizeof(rull->saddr_s));
	rull->saddr_e = (char *)malloc(sizeof(rull->saddr_e));
	rull->daddr_s = (char *)malloc(sizeof(rull->daddr_s));
	rull->daddr_e = (char *)malloc(sizeof(rull->daddr_e));

	//Rull 초기화.//
	rull->protocol = 0;
	strcpy(rull->saddr_s , IP_MIN_ALL_OPTION);
	strcpy(rull->saddr_e , IP_MAX_ALL_OPTION);
	rull->s_ip_accept_drop_check = ACCEPT_DROP_NONE;
	strcpy(rull->daddr_s , IP_MIN_ALL_OPTION);
	strcpy(rull->daddr_e , IP_MAX_ALL_OPTION);
	rull->d_ip_accept_drop_check = ACCEPT_DROP_NONE;
	rull->source_port = PORT_ALL_OPTION;
	rull->source_port_accept_drop_check = ACCEPT_DROP_NONE;
	rull->dest_port = PORT_ALL_OPTION;
	rull->dest_port_accept_drop_check = ACCEPT_DROP_NONE;

	for(controlbit_count = 0; controlbit_count < 6; controlbit_count++)
	{
		rull->set_controlbit_array[controlbit_count] = 0;
	}

	rull->set_controlbit_accept_drop_check = ACCEPT_DROP_NONE;

	sleep(2); //2초간 주어서 실제 사용자 UI를 고려.//
		
	printf("\033[%dm1. Initialization complete rules!!\033[0m\n", 35);
}
///////////////////////
void Rull_set(struct rull_set *rull)
{
	//시작주소, 목적지주소 설정.//
	rull->saddr_s = (char *)malloc(sizeof(rull->saddr_s));
	rull->saddr_e = (char *)malloc(sizeof(rull->saddr_e));
	rull->daddr_s = (char *)malloc(sizeof(rull->daddr_s));
	rull->daddr_e = (char *)malloc(sizeof(rull->daddr_e));

	//IP부분 변수.//
	int number;
	int source_port; //시작포트.//
	int dest_port; //도착포트.//
	int count = 1;
	int i;
	int protocol_set; //프로토콜 셋팅.//

	//출력할 데이터 값을 받기위한 문자열 변수.//
	char str_1[30];
	char str_2[30];
	char str_3[30];
	char str_4[30];
	char str_5[30];
	char str_6[30];
	char str_7[30];
	char str_8[30];
	char str_9[30];
	char str_10[30];
	char str_11[30];
	char str_12[30];

	//여러 셋팅 옵션.//
	int select_start_ip_address;
	int select_end_ip_address;
	int select_dest_port_option;
	int select_source_port_option;
	int select_controlbit_option;
	int select_protocol_case; 
	
	//ACCEPT_DROP 관련 변수.//
	int ip_accept_drop_check;
	int port_accept_drop_check;
	int control_bit_accept_drop_check;
	//파일에서 사용자가 원하는 룰을 읽어와서 적용하는 부분 알고리즘.//
	//셋팅순서는 Protocol -> IP -> PORT -> ControlBit 순으로 셋팅한다.셋팅의 방법은 우선적으로 화면이 클리어되고,
	//파일에 저장된 기록이 나오고 사용자가 적용할 룰을 선택하는것. 만약 새로운 룰 존재 시 사용자는 먼저 룰을 추가하고, 적용가능.//
	sleep(1); //2초간의 시간.//

	system("clear");

	printf("\033[%dm< 1st : Protocol part of the Rules >\n\033[0m", 36);
	printf("\n<< Protocol case >>\n");

	printf("1. TCP(default) / 2. UDP\n");
	
	printf("\033[%dm\n* Input protocol option : \033[0m", 31);
	scanf("%d", &select_protocol_case);

	//프로토콜 타입 결정.//
	if(select_protocol_case == 1)
	{
		rull->protocol = TCP_SET;
	}

	else if(select_protocol_case == 2)
	{
		rull->protocol = UDP_SET;
	}

	sleep(1);

	printf("\033[%dmAll PROTOCOL(TCP) Address Completed Apply options...\n\033[0m", 33);

	sleep(1);

	system("clear");
	
	printf("\033[%dm< 2st : IP part of the Rules >\n\033[0m", 36);
	printf("\n\033[%dm* No.1 is the basic band (default) source_range.\033[0m\n", 33);
	printf("\033[%dm* No.2 is the basic band (default) dest_range.\033[0m\n", 33);
	printf("\n<< Source IP Rull View >>\n");

	//첫번째 규칙은 IP적용이다.//

	//전체 정보를 다 보여주어야 하기에 파일 3개를 오픈.//
	if((ip_rull_set_file = fopen("ip_rull", "r")) == NULL)
	{
		fprintf(stderr, "ip rull file open error\n");
		exit(1);
	}

	while(!feof(ip_rull_set_file))
	{
		fscanf(ip_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, 				str_10);

		if(strcmp(str_5, "s_s/s_e") == 0)
		{
			printf("%d %s %s %s %s %s %s %s %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, str_10);
		}
	}

	printf("\033[%dm\n* Input ip option Number(s_s/s_e) : \033[0m", 31);
	scanf("%d", &select_start_ip_address);

	fseek(ip_rull_set_file, 0, SEEK_SET); //파일 포인터의 위치를 처음으로 돌린다.//

	while(!feof(ip_rull_set_file))
	{
		fscanf(ip_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, 				str_10);

		if(strcmp(str_5, "s_s/s_e") == 0 && count == select_start_ip_address) //시작주소에 대해서와 선택된 번호에 대해서 출력.//
		{
			break; //해당 지점에서 종료.//
		}

		count++; //다음 행으로 증가.//
	}

	if(strcmp(str_6, "DEFAULT_S") == 0)
	{
		strcpy(rull->saddr_s, IP_MIN_ALL_OPTION);
	}

	if(strcmp(str_8, "DEFAULT_E") == 0)
	{
		strcpy(rull->saddr_e, IP_MAX_ALL_OPTION);
	}

	else
	{
		strcpy(rull->saddr_s,str_6);
		strcpy(rull->saddr_e, str_8);
	}

	if(strcmp(str_9, "ACCEPT") == 0)
	{
		rull->s_ip_accept_drop_check = ACCEPT_OPTION;
	}

	if(strcmp(str_9, "DROP") == 0)
	{
		rull->s_ip_accept_drop_check = DROP_OPTION;
	}

	printf("\033[%dmstart IP Address Completed Apply options...\n\033[0m", 33);

	//도착지 IP주소를 설정.//
	fseek(ip_rull_set_file, 0, SEEK_SET); //파일 포인터의 위치를 처음으로 돌린다.//	
	count = 1;

	printf("\n<< dest IP Rull View >>\n");

	while(!feof(ip_rull_set_file))
	{
		fscanf(ip_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, 				str_10);

		if(strcmp(str_5, "d_s/d_e") == 0)
		{
			printf("%d %s %s %s %s %s %s %s %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, str_10);
		}
	}

	fseek(ip_rull_set_file, 0, SEEK_SET); //파일 포인터의 위치를 처음으로 돌린다.//	

	printf("\033[%dm\n* Input ip option Number(d_s/d_e) : \033[0m", 31);
	scanf("%d", &select_end_ip_address);

	while(!feof(ip_rull_set_file))
	{
		fscanf(ip_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, 				str_10);

		if(strcmp(str_5, "d_s/d_e") == 0 && count == select_end_ip_address) //시작주소에 대해서와 선택된 번호에 대해서 출력.//
		{
			break; //해당 지점에서 종료.//
		}

		count++; //다음 행으로 증가.//
	}

	if(strcmp(str_6, "DEFAULT_S") == 0)
	{
		strcpy(rull->daddr_s, IP_MIN_ALL_OPTION);
	}

	if(strcmp(str_8, "DEFAULT_E") == 0)
	{
		strcpy(rull->daddr_e, IP_MAX_ALL_OPTION);
	}

	else
	{
		strcpy(rull->daddr_s,str_6);
		strcpy(rull->daddr_e, str_8);
	}

	if(strcmp(str_9, "ACCEPT") == 0)
	{
		rull->d_ip_accept_drop_check = ACCEPT_OPTION;
	}

	if(strcmp(str_9, "DROP") == 0)
	{
		rull->d_ip_accept_drop_check = DROP_OPTION;
	}

	fclose(ip_rull_set_file);

	sleep(1);

	printf("\033[%dmAll IP Address Completed Apply options...\n\033[0m", 33);

	sleep(1);

	//도착지의 IP주소를 셋팅하기 위해서 다시 clear명령//
	system("clear");

	printf("\033[%dm< 3st : PORT part of the Rules >\n\033[0m", 36);
	printf("\n\033[%dm* No.1 is the basic source port number (default) range.\033[0m\n", 33);
	printf("\033[%dm* No.2 is the basic dest port number (default) range.\033[0m\n", 33);
	printf("\n<< Source PORT Rull View >>\n");

	if((port_rull_set_file = fopen("port_rull", "r")) == NULL)
	{
		fprintf(stderr, "port rull file open error\n");
		exit(1);
	}

	while(!feof(port_rull_set_file))
	{
		fscanf(port_rull_set_file, "%d %s %s %s %s %s %d %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, &source_port, 			str_6, str_7, str_8);

		if(strcmp(str_5, "sport") == 0)
		{
			printf("%d %s %s %s %s %s %d %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5,source_port,
			str_6, str_7, str_8);
		}
	}

	printf("\033[%dm\n* Input source port option Number(sport) : \033[0m", 31);
	scanf("%d", &select_source_port_option);

	fseek(port_rull_set_file, 0, SEEK_SET); //파일 포인터의 위치를 처음으로 돌린다.//
	count = 1;
	
	while(!feof(port_rull_set_file))
	{
		fscanf(port_rull_set_file, "%d %s %s %s %s %s %d %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, &source_port, 			str_6, str_7, str_8);

		if(select_source_port_option == count)
		{
			break;
		}

		count++;
	}

	if(strcmp(str_7, "ACCEPT") == 0)
	{
		rull->source_port_accept_drop_check = ACCEPT_OPTION;
	}

	if(strcmp(str_7, "DROP") == 0)
	{
		rull->source_port_accept_drop_check = DROP_OPTION;
	}

	rull->source_port = source_port;

	printf("\033[%dmSource PORT Address Completed Apply options...\n\033[0m", 33);

	sleep(1);

	fseek(port_rull_set_file, 0, SEEK_SET); //파일 포인터의 위치를 처음으로 돌린다.//
	count = 1;

	printf("\n<< Destination PORT Rull View >>\n");

	while(!feof(port_rull_set_file))
	{
		fscanf(port_rull_set_file, "%d %s %s %s %s %s %d %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, &dest_port, 			str_6, str_7, str_8);

		if(strcmp(str_5, "dport") == 0)
		{
			printf("%d %s %s %s %s %s %d %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5, dest_port,
			str_6, str_7, str_8);
		}
	}

	printf("\033[%dm\n* Input dest port option Number(dport) : \033[0m", 31);
	scanf("%d", &select_dest_port_option);

	fseek(port_rull_set_file, 0, SEEK_SET); //파일 포인터의 위치를 처음으로 돌린다.//
	count = 1;
	
	while(!feof(port_rull_set_file))
	{
		fscanf(port_rull_set_file, "%d %s %s %s %s %s %d %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, &dest_port, 			str_6, str_7, str_8);

		if(select_dest_port_option == count)
		{
			break;
		}

		count++;
	}

	if(strcmp(str_7, "ACCEPT") == 0)
	{
		rull->dest_port_accept_drop_check = ACCEPT_OPTION;
	}

	if(strcmp(str_7, "DROP") == 0)
	{
		rull->dest_port_accept_drop_check = DROP_OPTION;
	}

	rull->dest_port = dest_port;

	sleep(1);

	printf("\033[%dmAll PORT Address Completed Apply options...\n\033[0m", 33);

	sleep(1);

	system("clear");

	fclose(port_rull_set_file);

	printf("\033[%dm< 4st : Control-Bit part of the Rules >\n\033[0m", 36);
	printf("\n\033[%dm* No.1 is the basic band (default) range.\033[0m\n", 33);
	if((controlbit_rull_set_file = fopen("cb_rull", "r")) == NULL)
	{
		fprintf(stderr, "port rull file open error\n");
		exit(1);
	}
	
	printf("\n<< Control-Bit Rull View >>\n");

	while(!feof(controlbit_rull_set_file))
	{
		fscanf(controlbit_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, 			str_8, str_9, str_10, str_11, str_12);

		printf("%d %s %s %s %s %s %s %s %s %s %s %s %s\n", number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, str_8, str_9, str_10, 			str_11, str_12);
	}

	printf("\033[%dm\n* Input Control-Bit option : \033[0m", 31);
	scanf("%d", &select_controlbit_option);

	fseek(controlbit_rull_set_file, 0, SEEK_SET); //파일 포인터의 위치를 처음으로 돌린다.//
	count = 1;

	while(!feof(controlbit_rull_set_file))
	{
		fscanf(controlbit_rull_set_file, "%d %s %s %s %s %s %s %s %s %s %s %s %s\n", &number, str_1, str_2, str_3, str_4, str_5, str_6, str_7, 			str_8, str_9, str_10, str_11, str_12);

		if(count == select_controlbit_option)
		{
			break; //현재 찾은 위치에서 중지.//
		}

		count++;
	}

	if(select_controlbit_option == 1)
	{
		(strcmp(str_5, "ALL") == 0) ? (rull->set_controlbit_array[0] = -1) : (rull->set_controlbit_array[0] = -1);
		(strcmp(str_6, "ALL") == 0) ? (rull->set_controlbit_array[1] = -1) : (rull->set_controlbit_array[1] = -1);
		(strcmp(str_7, "ALL") == 0) ? (rull->set_controlbit_array[2] = -1) : (rull->set_controlbit_array[2] = -1);
		(strcmp(str_8, "ALL") == 0) ? (rull->set_controlbit_array[3] = -1) : (rull->set_controlbit_array[3] = -1);
		(strcmp(str_9, "ALL") == 0) ? (rull->set_controlbit_array[4] = -1) : (rull->set_controlbit_array[4] = -1);
		(strcmp(str_10,"ALL") == 0) ? (rull->set_controlbit_array[5] = -1) : (rull->set_controlbit_array[5] = -1);
	}

	else
	{
		//삼항연산자로 룰 배열 초기화.//
		(strcmp(str_5, "URG") == 0) ? (rull->set_controlbit_array[0] = 1) : (rull->set_controlbit_array[0] = 0);
		(strcmp(str_6, "ACK") == 0) ? (rull->set_controlbit_array[1] = 1) : (rull->set_controlbit_array[1] = 0);
		(strcmp(str_7, "PSH") == 0) ? (rull->set_controlbit_array[2] = 1) : (rull->set_controlbit_array[2] = 0);
		(strcmp(str_8, "RST") == 0) ? (rull->set_controlbit_array[3] = 1) : (rull->set_controlbit_array[3] = 0);
		(strcmp(str_9, "SYN") == 0) ? (rull->set_controlbit_array[4] = 1) : (rull->set_controlbit_array[4] = 0);
		(strcmp(str_10,"FIN") == 0) ? (rull->set_controlbit_array[5] = 1) : (rull->set_controlbit_array[5] = 0);
	}

	if(strcmp(str_11, "ACCEPT") == 0)
	{
		rull->set_controlbit_accept_drop_check = ACCEPT_OPTION;
	}

	if(strcmp(str_11, "DROP") == 0)
	{
		rull->set_controlbit_accept_drop_check = DROP_OPTION;
	}

	sleep(1);

	printf("\033[%dmAll Control-Bit Completed Apply options...\n\033[0m", 33);

	printf("\n");

	sleep(1);

	system("clear");
}
///////////////////////
void Read_Packet_Not_Rull(int raw_socket)
{
	printf("\n");

	printf("\033[%dm\nIf you exit, press the 'end'\033[0m\n", 31);///////////////////////////////////////////////////

	printf("\033[%dm\nRead Packet(Not Rull Mode) Start...\033[0m\n\n", 31);

	int len;

	int n = 0;/////////////////////////////////////////////////////////////////////////////////////////////////////////


	//전송되어지는 패킷은 ip+tcp패킷구조이므로 수신된 패킷의 사이즈는 각각의 헤더들에 대한(가변길이 데이터 포함.)
	//사이즈를 더하면 된다.//
	int rx_packet_size = sizeof(struct iphdr) + sizeof(struct tcphdr);
	char *rx_packet = malloc(rx_packet_size);

	struct tcphdr *rx_tcph; //수신된 tcp의 정보를 보기 위해서  tcphdr구조체 선언.//
	struct iphdr *rx_iph; //수신된 ip의 정보를 보기 위해서 iphdr구조체 선언.//

	struct in_addr s_addr, d_addr; //ip주소 설정을 위한 구조체.//
	struct sockaddr_in local, remote; //소켓설정을 위한  sockaddr_in구조체 설정.//

	struct servent *serv; // /etc/service에 존재하는 정보를 얻기 위해서 구조체 선언.//

	//비규칙 모드는 필터링한 결과가 전체 로그파일에만 남게 된다.//
	if((total_packet_log_file = fopen("TOTAL_PACKET.txt", "a")) == NULL)
	{
		fprintf(stderr, "file open error\n");
		exit(1);
	}

	while(1)
	{
		bzero(rx_packet, rx_packet_size);

		len = sizeof(local);


/////////////////////////////////////////////////////////////////////////////////////////////
		//raw소켓에 대해서 패킷을 받을때는 recvfrom()을 사용한다.//
		if((n = recvfrom(raw_socket, rx_packet, rx_packet_size, 0x0, (struct sockaddr *)&local, &len)) < 0)
		{
			printf("recvfrom error\n");
			exit(-2);

		}
		else if(n > 0 && play_start == 1)
		{			
			//현재 받은 패킷에 대해서 ip와 tcp를 구조체로 구분한다.//
			rx_iph = (struct iphdr *)(rx_packet);
			rx_tcph = (struct tcphdr *)(rx_packet + rx_iph->ihl * 4); //ip부분에서 헤더길이를(HL = HL*4)포함한 이후가 
			//tcp영역이 된다.즉 데이터부분이 tcp헤더가 오게되므로 기존 ip의 헤더사이즈를 더해야 한다.//

			Print_Packet_info(rx_iph, rx_tcph); //패킷 정보를 출력(비 옵션이기에 뷰어에 가까운 수준).//
		}

		else if(n > 0 && play_start == 0)
		{

			printf("\n");

			printf("\033[%dm\nPacket filtering exit...\033[0m\n\n", 31);

			if(total_packet_log_file != NULL)
			{
				fclose(total_packet_log_file); //파일을 닫는다.//
			}
			sleep(2);
			system("clear");

			break;

		}
/////////////////////////////////////////////////////////////////////////////////////////////

	}
}
///////////////////////////////
void Read_Packet_Rull(int raw_socket, struct rull_set *rull)
{
	int i;

	int n =0 ;//////////////////////////////////////////////////////////

	printf("\n");

	printf("\033[%dm\nIf you exit, press the 'end'\033[0m\n", 31);///////////////////////////////////////////////////

	printf("\033[%dm\nRead Packet(Rull Mode) Start...\033[0m\n\n", 31);

	int len;

	//전송되어지는 패킷은 ip+tcp패킷구조이므로 수신된 패킷의 사이즈는 각각의 헤더들에 대한(가변길이 데이터 포함.)
	//사이즈를 더하면 된다.//
	int rx_packet_size = sizeof(struct iphdr) + sizeof(struct tcphdr);
	char *rx_packet = malloc(rx_packet_size);

	struct tcphdr *rx_tcph; //수신된 tcp의 정보를 보기 위해서  tcphdr구조체 선언.//
	struct iphdr *rx_iph; //수신된 ip의 정보를 보기 위해서 iphdr구조체 선언.//

	struct in_addr s_addr, d_addr; //ip주소 설정을 위한 구조체.//
	struct sockaddr_in local, remote; //소켓설정을 위한  sockaddr_in구조체 설정.//

	struct servent *serv; // /etc/service에 존재하는 정보를 얻기 위해서 구조체 선언.//

	//룰 포함 필터링 관련 로그파일 오픈.//
	if((correct_packet_log_file = fopen("ACCEPT_PACKET.txt", "a")) == NULL)
	{
		fprintf(stderr, "file open error\n");
		exit(1);
	}

	if((incorrect_packet_log_file = fopen("DROP_PACKET.txt", "a")) == NULL)
	{
		fprintf(stderr, "file open error\n");
		exit(1);
	}

	if((total_packet_log_file = fopen("TOTAL_PACKET.txt", "a")) == NULL)
	{
		fprintf(stderr, "file open error\n");
		exit(1);
	}

	fputs("\n",incorrect_packet_log_file );
	fputs("\n",correct_packet_log_file );

	fputs("<<Firewall Rull Info>>",incorrect_packet_log_file );
	fputs("<<Firewall Rull Info>>",correct_packet_log_file );

	fputs("\n",incorrect_packet_log_file );
	fputs("\n",correct_packet_log_file );

	int controlbit_count = 0;

	fprintf(incorrect_packet_log_file, "* protocol : %d\n", rull->protocol);
	fprintf(correct_packet_log_file, "* protocol : %d\n", rull->protocol);

	fprintf(incorrect_packet_log_file,"* saddr_s : %s\n", rull->saddr_s);
	fprintf(correct_packet_log_file,"* saddr_s : %s\n", rull->saddr_s);

	fprintf(incorrect_packet_log_file,"* saddr_e : %s\n", rull->saddr_e);
	fprintf(correct_packet_log_file,"* saddr_e : %s\n", rull->saddr_e);

	fprintf(incorrect_packet_log_file,"* s_ip_accept_drop_check : %d\n", rull->s_ip_accept_drop_check);
	fprintf(correct_packet_log_file,"* s_ip_accept_drop_check : %d\n", rull->s_ip_accept_drop_check);

	fprintf(incorrect_packet_log_file,"* daddr_s : %s\n", rull->daddr_s);
	fprintf(correct_packet_log_file,"* daddr_s : %s\n", rull->daddr_s);

	fprintf(incorrect_packet_log_file,"* daddr_e : %s\n", rull->daddr_e);
	fprintf(correct_packet_log_file,"* daddr_e : %s\n", rull->daddr_e);

	fprintf(incorrect_packet_log_file,"* d_ip_accept_drop_check : %d\n", rull->d_ip_accept_drop_check);
	fprintf(correct_packet_log_file,"* d_ip_accept_drop_check : %d\n", rull->d_ip_accept_drop_check);

	fprintf(incorrect_packet_log_file,"* source_port : %d\n", rull->source_port);
	fprintf(correct_packet_log_file,"* source_port : %d\n", rull->source_port);

	fprintf(incorrect_packet_log_file,"* source_port_accept_drop_check : %d\n", rull->source_port_accept_drop_check);
	fprintf(correct_packet_log_file,"* source_port_accept_drop_check : %d\n", rull->source_port_accept_drop_check);

	fprintf(incorrect_packet_log_file,"* dest_port : %d\n", rull->dest_port);
	fprintf(correct_packet_log_file,"* dest_port : %d\n", rull->dest_port);

	fprintf(incorrect_packet_log_file,"* dest_port_accept_drop_check : %d\n", rull->dest_port_accept_drop_check);
	fprintf(correct_packet_log_file,"* dest_port_accept_drop_check : %d\n", rull->dest_port_accept_drop_check);

	fprintf(incorrect_packet_log_file,"* ControlBit(URG/ACK/PSH/RST/SYN/FIN) : ");
	fprintf(correct_packet_log_file,"* ControlBit(URG/ACK/PSH/RST/SYN/FIN) : ");

	for(controlbit_count = 0; controlbit_count < 6; controlbit_count++)
	{
		fprintf(incorrect_packet_log_file,"%d ", rull->set_controlbit_array[controlbit_count]);
		fprintf(correct_packet_log_file,"%d ", rull->set_controlbit_array[controlbit_count]);
	}

	fputs("\n",incorrect_packet_log_file);
	fputs("\n",correct_packet_log_file);

	fprintf(incorrect_packet_log_file,"* controlbit_accept_drop_check : %d\n", rull->set_controlbit_accept_drop_check);
	fprintf(correct_packet_log_file,"* controlbit_accept_drop_check : %d\n", rull->set_controlbit_accept_drop_check);

	while(1)
	{
		bzero(rx_packet, rx_packet_size);

		len = sizeof(local);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//raw소켓에 대해서 패킷을 받을때는 recvfrom()을 사용한다.//
		if((n = recvfrom(raw_socket, rx_packet, rx_packet_size, 0x0, (struct sockaddr *)&local, &len)) < 0)
		{
			printf("recvfrom error\n");
			exit(-2);
		}
		else if(n > 0 && play_start == 1)
		{
			//현재 받은 패킷에 대해서 ip와 tcp를 구조체로 구분한다.//
			rx_iph = (struct iphdr *)(rx_packet);
			rx_tcph = (struct tcphdr *)(rx_packet + rx_iph->ihl * 4); //ip부분에서 헤더길이를(HL = HL*4)포함한 이후가 
			//tcp영역이 된다.즉 데이터부분이 tcp헤더가 오게되므로 기존 ip의 헤더사이즈를 더해야 한다.//

			Check_Packet_info(rx_iph, rx_tcph, rull); //패킷 정보를 출력(옵션이기에 규칙관련 구조체 적용)//
		}
		else if(n > 0 && play_start == 0)
		{

			printf("\n");

			printf("\033[%dm\nPacket filtering exit...\033[0m\n\n", 31);

			if(correct_packet_log_file != NULL)
			{
				fclose(correct_packet_log_file);
			}

			if(incorrect_packet_log_file != NULL)
			{
				fclose(incorrect_packet_log_file);
			}

			if(total_packet_log_file != NULL)
			{
				fclose(total_packet_log_file); //파일을 닫는다.//
			}
			sleep(2);
			system("clear");

			break;
		}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
}
///////////////////////////////
void Print_Packet_info(struct iphdr *iph, struct tcphdr *tcph) //패킷정보 출력.(룰을 적용하지 않음.)//
{
	fputs("\n", total_packet_log_file);

	//IP부분 관련 정보출력.//
	//이렇게 직접 구조체의 값을 가져올 수 있으므로 해킹(조작)이 가능하다.//
	printf("[IP HEADER] VER: %1u HL : %2u Protocol : %3u ", iph->version, iph->ihl, iph->protocol);
	printf("SRC IP : %15s ", inet_ntoa(*(struct in_addr *)&iph->saddr)); //Source IP출력.//
	printf("DEST IP : %15s \n", inet_ntoa(*(struct in_addr *)&iph->daddr)); //Destination IP출력.//

	//TCP관련 정보출력.//
	//이렇게 TCP관련 정보도 직접 구조체에 접근하여서 해킹(수정)이 가능하다.//
	printf("[TCP HEADER] src port: %5u dest port : %5u ", ntohs(tcph->source), ntohs(tcph->dest));

	//삼항연산자로 제어필드 구분.//
	//네트워크 바이트 순서는 빅 엔디안이기에 URG부터 처리.//
	(tcph->urg == 1) ? printf("U") : printf("-"); //Urgent가 설정되있으면 표시, 아니면 '-'표시.//
	(tcph->ack == 1) ? printf("A") : printf("-");
	(tcph->psh == 1) ? printf("P") : printf("-");
	(tcph->rst == 1) ? printf("R") : printf("-");
	(tcph->syn == 1) ? printf("S") : printf("-");
	(tcph->fin == 1) ? printf("F") : printf("-");

	printf("\n\n");

	timer = time(NULL); //현재 시각을 초 단위로 얻기.//
	t = localtime(&timer); //초 단위의 시간을 분리하여 구조체에 넣는다.//

	fputs("* Packet trace Access time : ", total_packet_log_file);									
				
	fprintf(total_packet_log_file, "%d.%d.%d %d:%d:%d\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	//파일저장 부분. 이 부분에 로그파일은 전체로그파일에 들어가게 된다.//
	fprintf(total_packet_log_file,"[IP HEADER] VER: %u HL : %u Protocol : %u ", iph->version, iph->ihl, iph->protocol);
	fprintf(total_packet_log_file,"SRC IP : %s ", inet_ntoa(*(struct in_addr *)&iph->saddr)); //Source IP출력.//
	fprintf(total_packet_log_file,"DEST IP : %s \n", inet_ntoa(*(struct in_addr *)&iph->daddr)); //Destination IP출력.//

	fprintf(total_packet_log_file,"[TCP HEADER] src port: %u dest port : %u ", ntohs(tcph->source), ntohs(tcph->dest));
	
	(tcph->urg == 1) ? fprintf(total_packet_log_file,"U") : fprintf(total_packet_log_file,"-"); //Urgent가 설정되있으면 표시, 아니면 '-'표시.//
	(tcph->ack == 1) ? fprintf(total_packet_log_file,"A") : fprintf(total_packet_log_file,"-");
	(tcph->psh == 1) ? fprintf(total_packet_log_file,"P") : fprintf(total_packet_log_file,"-");
	(tcph->rst == 1) ? fprintf(total_packet_log_file,"R") : fprintf(total_packet_log_file,"-");
	(tcph->syn == 1) ? fprintf(total_packet_log_file,"S") : fprintf(total_packet_log_file,"-");
	(tcph->fin == 1) ? fprintf(total_packet_log_file,"F") : fprintf(total_packet_log_file,"-");

	fputs("\n", total_packet_log_file);
	fputs("-----------------------------------", total_packet_log_file);
}
///////////////////////////////
void Check_Packet_info(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull) //패킷정보 출력.(룰을 적용해서 판단.)//
{
	int rull_protocol_check_value; //프로토콜 체크 관련 변수. 프로토콜이 아예 다르면 의미가 없기에 0,1로만 구분.//
	int rull_check_value; //검사를 하고 반환된 반환값.-1이면 바로 해당 에러메시지와 실패로그로 기록이되게 된다.(DROP 시 -1)//

	//여러 종류의 에러메시지 선언.//
	char protocol_error_msg[80] = "# ERROR PACKET : This is a violation of the protocol specified in the rule."; 
	char source_ip_error_msg[80] = "# ERROR PACKET : Firewall rules are violated to an Source IP address range.";
	char dest_ip_error_msg[80] = "# ERROR PACKET : Firewall rules are violated to an Dest IP address range.";
	char source_port_error_msg[80] = "# ERROR PACKET : It is a violation of the rules Source port number.";
	char dest_port_error_msg[80] = "# ERROR PACKET : It is a violation of the rules Dest port number.";
	char controlbit_error_msg[80] = "# ERROR PACKET : Control Bit is a violation of that rule..";

	multi_error_message = (char *)malloc(strlen("Input")+1);

	printf("------------------<<Packet Info>>----------------------\n");
	//IP부분 관련 정보출력.//
	//이렇게 직접 구조체의 값을 가져올 수 있으므로 해킹(조작)이 가능하다.//
	printf("[IP HEADER] VER: %1u HL : %2u Protocol : %3u ", iph->version, iph->ihl, iph->protocol);
	printf("SRC IP : %15s ", inet_ntoa(*(struct in_addr *)&iph->saddr)); //Source IP출력.//
	printf("DEST IP : %15s \n", inet_ntoa(*(struct in_addr *)&iph->daddr)); //Destination IP출력.//

	//TCP관련 정보출력.//
	//이렇게 TCP관련 정보도 직접 구조체에 접근하여서 해킹(수정)이 가능하다.//
	printf("[TCP HEADER] src port: %5u dest port : %5u ", ntohs(tcph->source), ntohs(tcph->dest));

	//삼항연산자로 제어필드 구분.//
	//네트워크 바이트 순서는 빅 엔디안이기에 URG부터 처리.//
	(tcph->urg == 1) ? printf("U") : printf("-"); //Urgent가 설정되있으면 표시, 아니면 '-'표시.//
	(tcph->ack == 1) ? printf("A") : printf("-");
	(tcph->psh == 1) ? printf("P") : printf("-");
	(tcph->rst == 1) ? printf("R") : printf("-");
	(tcph->syn == 1) ? printf("S") : printf("-");
	(tcph->fin == 1) ? printf("F") : printf("-");

	printf("\n*******************************************************");
	printf("\n* Result Anlaysis\n"); 

	//검사의 순서는 Protocol -> IP -> PORT -> CB//
	//프로토콜 비교.//

	rull_protocol_check_value = Rull_Check_protocol(iph, tcph, rull); //프로토콜 검사.0이면 UDP, 1이면 TCP이다.//

	if(rull_protocol_check_value == -1)
	{
		//drop관련 로그파일에 저장.//
		save_drop_log_file(iph, tcph, rull,protocol_error_msg); //메시지 저장.//
	}

	//패킷 검사. 1st. IP검사 -> 2st. PORT비교 -> 3st. ControlBit비교.//
	else if(rull_protocol_check_value == 1) //TCP인 경우 -> TCP만의 헤더구조로 나머지 정보들 비교가능.//
	{
		//각 경우의 수에 따른 구분.//
		if(rull_check_count == 0) //옵션이 없음.바로 전체로그에 저장.//
		{
			save_total_log_file(iph, tcph, rull);
		}

		else if(rull_check_count == 1) //Source IP만 적용.//
		{	
			//현재 source_ip부분에 대한 검사이기에 source_ip에 적용된 accept,drop의 경우를 먼저 나누어준다.//
			//즉 accept에서의 반환값과 drop의 경우에 반환값이 인터페이스상 다르기에 구분지어서 판단 시 오인이 없도록 한다.//

			if(rull->s_ip_accept_drop_check == 1) //ACCEPT인 경우.//
			{
				rull_check_value = Rull_Check_source_ip(iph, tcph, rull); //ip검사.//	

				if(rull_check_value == -1) //룰에 조건에 의해 부합하지 않는 경우. 바로 폐기하고 DROP로그에 작성된다.조건을 이어나갈 수 없다.//
				{
					printf("ERROR PACKET\n");
			
					save_drop_log_file(iph, tcph, rull,source_ip_error_msg ); //메시지 저장.//
				}

				else if(rull_check_value = 1) //룰에 판단에 의한 정상적인 경우.조건의 연속성이 있으면 계속 이어나갈 수 있다.//
				{
					printf("SUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}

			else if(rull->s_ip_accept_drop_check == 0) //DROP인 경우.//
			{
				rull_check_value = Rull_Check_source_ip(iph, tcph, rull); //ip검사.//	

				if(rull_check_value == -1) //룰에 조건에 의해 부합하지 않는 경우.조건을 이어나갈 수 없다.//
				{
					printf("ERROR PACKET\n");
					
					save_drop_log_file(iph, tcph, rull, source_ip_error_msg); //메시지 저장.//
				}

				else if(rull_check_value = 1) //룰에 판단에 의한 정상적인 경우.DROP이 아니어야지 조건을 이어나갈 수 있다.//
				{
					printf("SUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}

			//우선 패킷의 정보를 전체 로그파일에 저장.//
			save_total_log_file(iph, tcph, rull);
		}

		else if(rull_check_count == 2) //Destination ip 적용.//
		{
			//현재 source_ip부분에 대한 검사이기에 source_ip에 적용된 accept,drop의 경우를 먼저 나누어준다.//
			//즉 accept에서의 반환값과 drop의 경우에 반환값이 인터페이스상 다르기에 구분지어서 판단 시 오인이 없도록 한다.//

			if(rull->d_ip_accept_drop_check == 1) //ACCEPT인 경우.//
			{
				rull_check_value = Rull_Check_dest_ip(iph, tcph, rull); //ip검사.//	

				if(rull_check_value == -1) //룰에 조건에 의해 부합하지 않는 경우. 바로 폐기하고 DROP로그에 작성된다.조건을 이어나갈 수 없다.//
				{
					printf("ERROR PACKET\n");
					
					save_drop_log_file(iph, tcph, rull, dest_ip_error_msg); //메시지 저장.//
				}

				else if(rull_check_value = 1) //룰에 판단에 의한 정상적인 경우.조건의 연속성이 있으면 계속 이어나갈 수 있다.//
				{
					printf("SUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}

			else if(rull->d_ip_accept_drop_check == 0) //DROP인 경우.//
			{
				rull_check_value = Rull_Check_dest_ip(iph, tcph, rull); //ip검사.//	

				if(rull_check_value == -1) //룰에 조건에 의해 부합하지 않는 경우.조건을 이어나갈 수 없다.//
				{
					printf("ERROR PACKET\n");
					
					save_drop_log_file(iph, tcph, rull, dest_ip_error_msg); //메시지 저장.//
				}

				else if(rull_check_value = 1) //룰에 판단에 의한 정상적인 경우.DROP이 아니어야지 조건을 이어나갈 수 있다.//
				{
					printf("SUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}

			//우선 패킷의 정보를 전체 로그파일에 저장.//
			save_total_log_file(iph, tcph, rull);
		}

		else if(rull_check_count == 3) //SP//
		{
			if(rull->source_port_accept_drop_check == 1) //ACCEPT 경우.//
			{
				rull_check_value =  Rull_Check_Source_port(iph, tcph, rull); //포트검사.//

				if(rull_check_value == -1) //-1의 의미는 ACCEPT시 허용되지 않는것이고, DROP시 차단되는 경우이다.//
				{
					printf("ERROR PACKET\n");

					save_drop_log_file(iph, tcph, rull, source_port_error_msg); //메시지 저장.//
				}

				else if(rull_check_value == 1) //1의 반환 의미는 ACCEPT시 허용되는것이고 DROP 시 drop이 아닌경우를 의미.// 
				{
					printf("SUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}

			else if(rull->source_port_accept_drop_check == 0) //DROP 경우.//
			{
				rull_check_value =  Rull_Check_Source_port(iph, tcph, rull); //포트검사.//

				if(rull_check_value == -1) //-1의 의미는 ACCEPT시 허용되지 않는것이고, DROP시 차단되는 경우이다.//
				{
					printf("ERROR PACKET\n");

					save_drop_log_file(iph, tcph, rull, source_port_error_msg); //메시지 저장.//
				}

				else if(rull_check_value == 1) //1의 반환 의미는 ACCEPT시 허용되는것이고 DROP 시 drop이 아닌경우를 의미.// 
				{
					printf("SUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}	

			//전체 패킷로그에 저장.//
			save_total_log_file(iph, tcph, rull);
		}

		else if(rull_check_count == 4) //DP//
		{
			if(rull->dest_port_accept_drop_check == 1) //ACCEPT 경우.//
			{
				rull_check_value =  Rull_Check_Dest_port(iph, tcph, rull); //포트검사.//

				if(rull_check_value == -1) //-1의 의미는 ACCEPT시 허용되지 않는것이고, DROP시 차단되는 경우이다.//
				{
					printf("ERROR PACKET\n");

					save_drop_log_file(iph, tcph, rull, dest_port_error_msg); //메시지 저장.//
				}

				else if(rull_check_value == 1) //1의 반환 의미는 ACCEPT시 허용되는것이고 DROP 시 drop이 아닌경우를 의미.// 
				{
					printf("SUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}

			else if(rull->dest_port_accept_drop_check == 0) //DROP 경우.//
			{
				rull_check_value =  Rull_Check_Dest_port(iph, tcph, rull); //포트검사.//

				if(rull_check_value == -1) //-1의 의미는 ACCEPT시 허용되지 않는것이고, DROP시 차단되는 경우이다.//
				{
					printf("ERROR PACKET\n");

					save_drop_log_file(iph, tcph, rull, dest_port_error_msg); //메시지 저장.//
				}

				else if(rull_check_value == 1) //1의 반환 의미는 ACCEPT시 허용되는것이고 DROP 시 drop이 아닌경우를 의미.// 
				{
					printf("SUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}	

			//전체 패킷로그에 저장.//
			save_total_log_file(iph, tcph, rull);
		}

		else if(rull_check_count == 5) //CB//
		{
			if(rull->set_controlbit_accept_drop_check == 1) //ACCEPT 경우.//
			{
				rull_check_value =  Rull_Check_ControlBit(iph, tcph, rull); //포트검사.//

				if(rull_check_value == -1) //-1의 의미는 ACCEPT시 허용되지 않는것이고, DROP시 차단되는 경우이다.//
				{
					printf("\nERROR PACKET\n");

					save_drop_log_file(iph, tcph, rull, controlbit_error_msg); //메시지 저장.//
				}

				else if(rull_check_value == 1) //1의 반환 의미는 ACCEPT시 허용되는것이고 DROP 시 drop이 아닌경우를 의미.// 
				{
					printf("\nSUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}

			else if(rull->set_controlbit_accept_drop_check == 0) //DROP 경우.//
			{
				rull_check_value =  Rull_Check_ControlBit(iph, tcph, rull); //포트검사.//

				if(rull_check_value == -1) //-1의 의미는 ACCEPT시 허용되지 않는것이고, DROP시 차단되는 경우이다.//
				{
					printf("\nERROR PACKET\n");

					save_drop_log_file(iph, tcph, rull, controlbit_error_msg); //메시지 저장.//
				}

				else if(rull_check_value == 1) //1의 반환 의미는 ACCEPT시 허용되는것이고 DROP 시 drop이 아닌경우를 의미.// 
				{
					printf("\nSUCCESS PACKET\n");

					save_accept_log_file(iph, tcph, rull); //메시지 저장.//
				}
			}	

			//전체 패킷로그에 저장.//
			save_total_log_file(iph, tcph, rull);
		}

		else if(rull_check_count == 6) //SI+DI//
		{
			//근원지 주소와 목적지 주소의 결합.//
			
		}
		
		else if(rull_check_count == 7) //SI+SP//
		{

		}

		else if(rull_check_count == 8) //SI+DP//
		{

		}

		else if(rull_check_count == 9) //SI+CB//
		{

		}

		else if(rull_check_count == 10) //DI+SP//	
		{

		}

		else if(rull_check_count == 11) //DI+DP//
		{

		}

		else if(rull_check_count == 12) //DI+CB//
		{

		}

		else if(rull_check_count == 13) //SP+DP//
		{

		}

		else if(rull_check_count == 14) //SP+CB//
		{

		}

		else if(rull_check_count == 15) //DP+CB//
		{

		}

		else if(rull_check_count == 16) //SI+DI+SP//
		{

		}

		else if(rull_check_count == 17) //SI+DI+DP//
		{

		}

		else if(rull_check_count == 18) //SI+DI+CB//
		{

		}

		else if(rull_check_count == 19) //SI+SP+DP//
		{

		}

		else if(rull_check_count == 20) //SI+SP+CB//
		{

		}

		else if(rull_check_count == 21) //SI+DP+CB//
		{

		}

		else if(rull_check_count == 22) //DI+SP+DP//
		{

		}

		else if(rull_check_count == 23) //DI+DP+CB//
		{

		}

		else if(rull_check_count == 24) //SP+DP+CB//
		{

		}

		else if(rull_check_count == 25) //DI+SP+CB//
		{

		}

		else if(rull_check_count == 26) //SI+DI+SP+DP//
		{

		}

		else if(rull_check_count == 27) //SI+DI+SP+CB//
		{

		}

		else if(rull_check_count == 28) //SI+DI+DP+CB//
		{

		}

		else if(rull_check_count == 29) //DI+SP+DP+CB//
		{

		}

		else if(rull_check_count == 30) //SI+SP+DP+CB//
		{

		}

		else if(rull_check_count == 31) //SI+DI+SP+DP+CB//
		{

		}
	} 
	
	printf("\n*******************************************************\n");
	printf("-------------------------------------------------------\n");

	printf("\n");

	//메모리 초기화.//
	//memset(source_ip_error_msg, 0, sizeof(source_ip_error_msg));
	//memset(dest_ip_error_msg, 0, sizeof(dest_ip_error_msg));

	//파일저장 부분.이 부분에서는 조건 기준으로 실패와 성공 로그파일로 각각 저장된다.//
}
///////////////////////////////
//protocol을 검사.//
int Rull_Check_protocol(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull)
{
	int check_value; //-1을 반환한다는 것을 DROP인 경우.1이면 ACCEPT로 다른 옵션이 있을 시 진행할 필요가 있음. drop일 시 차단의 개념이므로 더 이상 진행할 필요가 없으니 반환.//
	
	/* Protocol part ERROR MESSAGE
	"# ERROR PACKET : The rules of the protocol is incorrect."
	strcpy(error_message , "# ERROR PACKET : The rules of the protocol is incorrect.");
	*/

	//프로토콜 관련 변수.//
	int rull_protocol;
	int input_protocol;

	rull_protocol = rull->protocol;

	if(rull_protocol == iph->protocol)
	{
		check_value = 1; //TCP일 경우.//
	}

	else //기본적으로 TCP이므로 UDP이면 DROP.//
	{
		check_value = -1; //UDP일 경우.//
	}

	return check_value;
}
//////////////////////////////
//Source ip를 검사.//
int Rull_Check_source_ip(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull)
{
	int check_value; //현재 조건에 만족하면 1을 반환, 하나라도 만족하지 않으면(DROP) 바로 버려야 하기에(IP->PORT->ControlBit) -1을 반환.//
	//-1을 반환하게 되면 해당 검사함수에서 내보낼 수 있는 예외처리문을 채워서(Call by Reference이용) 같이 반환되게 된다.//
	int array_count = 0;
	int ip_check_count = 0; //이 카운트가 4가되면 ACCEPT, DROP에 따라서 반환값이 결정된다.//
	int i;

	/* Protocol part ERROR MESSAGE
	"# ERROR PACKET : The rules of the protocol is incorrect."
	strcpy(error_message , "# ERROR PACKET : Firewall rules are violated to an IP address range.");
	*/

	//비교를 위해서 3개의 ip주소의 x.y.z.w의 값을 배열로 유지하고 있는다.//
	int rull_source_start_ip_address_array[4]; //사이즈가 4로 고정인 이유는 ip의 주소구조는 x.y.z.w로 되어있다.//
	int rull_source_end_ip_address_array[4];
	int input_source_ip_address_array[4];

	//현재 룰을 가지고 설정.필요한 변수들 선언.//
	//근원지(Source)주소와 목적지(Destination)주소당 대역이 있으니 총 6개의 주소저장소가 필요.//
	//아래 각각의 int타입은 strtok을 위한 저장변수의 개념.//
	char *rull_source_start_ip = NULL;
	int rull_source_start_ip_address_1;
	int rull_source_start_ip_address_2;
	int rull_source_start_ip_address_3;
	int rull_source_start_ip_address_4;

	char *rull_source_end_ip = NULL;
	int rull_source_end_ip_address_1;
	int rull_source_end_ip_address_2;
	int rull_source_end_ip_address_3;
	int rull_source_end_ip_address_4;

	char *input_source_ip = NULL;
	int input_source_ip_address_1;
	int input_source_ip_address_2;
	int input_source_ip_address_3;
	int input_source_ip_address_4;

	//우선적으로 해당 옵션이 ACCEPT인지 DROP인지 판단.//
	//ip의 대역은 근원지 주소와 목적지 주소 2개로 이루어져 있다.//
	//룰에 정의된 값이 변경되면 안되기에 다른곳에 저장.//
	rull_source_start_ip = (char *)malloc(strlen("Input")+1);
	rull_source_end_ip = (char *)malloc(strlen("Input")+1);
	input_source_ip = (char *)malloc(strlen("Input")+1);

	strcpy(rull_source_start_ip, rull->saddr_s);
	strcpy(rull_source_end_ip, rull->saddr_e);  
	strcpy(input_source_ip, inet_ntoa(*(struct in_addr *)&iph->saddr));
	
	printf("- rull source start ip : %s / end ip : %s\n", rull_source_start_ip, rull_source_end_ip);
	printf("- input source ip : %s\n", input_source_ip);

	//Source IP부분에 대한 판단. 하나의 들어온 Source IP주소에 대해서 범위를 고려.//
	//우선은 3개의 ip를 각각 분할한다.//
	char *token; //토큰.//
	char *delim = "."; //구분자.//

	//3개의 IP주소대역을 각각 정수형 배열에 저장하기 위해서 처리해주는 작업.//
	token = strtok(rull_source_start_ip, delim);
	
	//printf("%s ", token);
	rull_source_start_ip_address_array[array_count] = atoi(token);

	while(token!=NULL)
	{
		token = strtok(NULL, delim);

		if(token == NULL)
		{
			break;
		}

		else
		{
			array_count++;
			//printf("%s ", token);
			rull_source_start_ip_address_array[array_count] = atoi(token); //정수형을 저장.//
		}
	}

	array_count = 0; //다음 ip주소대역을 저장하기 위해서 카운터를 초기화.//

	/*for(i=0; i<4; i++)
	{
		printf("%d ", rull_source_start_ip_address_array[i]);
	}*/

	printf("\n");

	token = strtok(rull_source_end_ip, delim);

	rull_source_end_ip_address_array[array_count] = atoi(token);

	while(token != NULL)
	{
		token = strtok(NULL, delim);

		if(token == NULL)
		{
			break;
		}

		else
		{
			array_count++;
			//printf("%s ", token);
			rull_source_end_ip_address_array[array_count] = atoi(token); //정수형을 저장.//
		}
	}

	/*for(i=0; i<4; i++)
	{
		printf("%d ", rull_source_end_ip_address_array[i]);
	}*/

	//printf("\n");

	array_count = 0; //다음 ip주소대역을 저장하기 위해서 카운터를 초기화.//

	token = strtok(input_source_ip, delim);

	input_source_ip_address_array[array_count] = atoi(token);

	while(token != NULL)
	{
		token = strtok(NULL, delim);

		if(token == NULL)
		{
			break;
		}

		else
		{
			array_count++;
			//printf("%s ", token);
			input_source_ip_address_array[array_count] = atoi(token); //정수형을 저장.//
		}
	}

	/*for(i=0; i<4; i++)
	{
		printf("%d ", input_source_ip_address_array[i]);
	}*/

	//비교할 3개의 IP주소대역을 나누었으므로 비교/판단 시작.//
	//현재 들어온 IP주소가 설정한 대역에 ACCEPT옵션이면 1을 반환, DROP옵션에 포함되면 -1을 반환.//
	//비교방법은 기준 ip(input ip)로 양 옆에 min과 max의 주소는 start, end의 IP주소로 한다.//
	//우선은 DROP모드인지 ACCEPT모드인지 구분.//

	//기본적으로 안되는 IP대역(256.0.0.0 ~)은 걸러낸다//
	if(input_source_ip_address_array[0] >= ERROR_IP_EXCEPTION) //기본적으로 IP대역이 256.이 넘으면 에러처리.//
	{
		check_value = -1;

		return check_value; //DROP의 경우랑 같으므로 -1을 반환.//
	}

	//ACCEPT, DROP에 유무에 따른 구분.반환 시 인터페이스의 유지가 필요.(ACCEPT -> 1, DROP -> -1)//
	//단 DROP으로 계속 갈 시 에러문을 추가해주어야 한다.//
	if(rull->s_ip_accept_drop_check == 1) //ACCEPT 모드//
	{
		printf("ACCEPT Mode apply");

		for(i=0; i<4; i++)
		{
			//첫번째 IP주소 범위(대역)에 대한 범위판단.범위이기에 대역시작 IP주소, 입력주소, 대역 마지막 IP주소로 비교한다.//
			if(rull_source_start_ip_address_array[i] <= input_source_ip_address_array[i] 
				&& input_source_ip_address_array[i] <= rull_source_end_ip_address_array[i])
			{
				//계속 true의 값을 유지하고 있는다.//
				ip_check_count++;
			}
		}

		//ACCEPT는 1을 반환 즉 그 범위안에 들었을때만 다음 조건을 진행가능.//
		if(ip_check_count == 4) //4개가 다 만족할 경우.현재 모드가 ACCEPT이므로 이 부분은 ACCEPT으로 간다.//
		{	
			check_value = 1;
		}

		else //1개의 영역이라도 만족하지 않을 경우. 이 경우는 현재 ACCEPT으로 보았을 시 거짓이므로 -1을 반환. DROP으로 간다.//
		{
			check_value = -1;
		}
	}

	else if(rull->s_ip_accept_drop_check == 0) //DROP모드.//
	{
		printf("DROP Mode apply");

		//배열 3개를 다루게 된다.//
		for(i=0; i<4; i++)
		{
			//첫번째 IP주소 범위(대역)에 대한 범위판단.범위이기에 대역시작 IP주소, 입력주소, 대역 마지막 IP주소로 비교한다.//
			if(rull_source_start_ip_address_array[i] <= input_source_ip_address_array[i] 
				&& input_source_ip_address_array[i] <= rull_source_end_ip_address_array[i])
			{
				//계속 true의 값을 유지하고 있는다.//
				ip_check_count++;
			}
		}

		if(ip_check_count == 4) //4개가 다 만족할 경우.현재 모드가 DROP이므로 이 부분은 DROP로그로 간다.계속 진행할 수 있다.//
		{	
			check_value = -1; 
		}

		else //1개의 영역이라도 만족하지 않을 경우. 이 경우는 현재 DROP으로 보았을 ACCEPT모드로 가는게 타당하다.이 또한 중첩해서 계속 조건을 진행할 수 있다.//
		{
			check_value = 1;
		}
	}

	printf("\n");

	return check_value;
}
////////////////////////////////
//Dest ip를 검사.//
int Rull_Check_dest_ip(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull)
{
	int check_value; //현재 조건에 만족하면 1을 반환, 하나라도 만족하지 않으면(DROP) 바로 버려야 하기에(IP->PORT->ControlBit) -1을 반환.//
	//-1을 반환하게 되면 해당 검사함수에서 내보낼 수 있는 예외처리문을 채워서(Call by Reference이용) 같이 반환되게 된다.//
	int array_count = 0;
	int ip_check_count = 0; //이 카운트가 4가되면 ACCEPT, DROP에 따라서 반환값이 결정된다.//
	int i;

	/* Protocol part ERROR MESSAGE
	"# ERROR PACKET : The rules of the protocol is incorrect."
	strcpy(error_message , "# ERROR PACKET : Firewall rules are violated to an IP address range.");
	*/

	//비교를 위해서 3개의 ip주소의 x.y.z.w의 값을 배열로 유지하고 있는다.//
	int rull_dest_start_ip_address_array[4]; //사이즈가 4로 고정인 이유는 ip의 주소구조는 x.y.z.w로 되어있다.//
	int rull_dest_end_ip_address_array[4];
	int input_dest_ip_address_array[4];

	//현재 룰을 가지고 설정.필요한 변수들 선언.//
	//근원지(Source)주소와 목적지(Destination)주소당 대역이 있으니 총 6개의 주소저장소가 필요.//
	//아래 각각의 int타입은 strtok을 위한 저장변수의 개념.//
	char *rull_dest_start_ip = NULL;
	int rull_dest_start_ip_address_1;
	int rull_dest_start_ip_address_2;
	int rull_dest_start_ip_address_3;
	int rull_dest_start_ip_address_4;

	char *rull_dest_end_ip = NULL;
	int rull_dest_end_ip_address_1;
	int rull_dest_end_ip_address_2;
	int rull_dest_end_ip_address_3;
	int rull_dest_end_ip_address_4;

	char *input_dest_ip = NULL;
	int input_dest_ip_address_1;
	int input_dest_ip_address_2;
	int input_dest_ip_address_3;
	int input_dest_ip_address_4;

	//우선적으로 해당 옵션이 ACCEPT인지 DROP인지 판단.//
	//ip의 대역은 근원지 주소와 목적지 주소 2개로 이루어져 있다.//
	//룰에 정의된 값이 변경되면 안되기에 다른곳에 저장.//
	rull_dest_start_ip = (char *)malloc(strlen("Input")+1);
	rull_dest_end_ip = (char *)malloc(strlen("Input")+1);
	input_dest_ip = (char *)malloc(strlen("Input")+1);

	strcpy(rull_dest_start_ip, rull->daddr_s);
	strcpy(rull_dest_end_ip, rull->daddr_e);  
	strcpy(input_dest_ip, inet_ntoa(*(struct in_addr *)&iph->daddr));
	
	printf("- rull dest start ip : %s / end ip : %s\n", rull_dest_start_ip, rull_dest_end_ip);
	printf("- input source ip : %s\n", input_dest_ip);

	//Source IP부분에 대한 판단. 하나의 들어온 Source IP주소에 대해서 범위를 고려.//
	//우선은 3개의 ip를 각각 분할한다.//
	char *token; //토큰.//
	char *delim = "."; //구분자.//

	//3개의 IP주소대역을 각각 정수형 배열에 저장하기 위해서 처리해주는 작업.//
	token = strtok(rull_dest_start_ip, delim);
	
	//printf("%s ", token);
	rull_dest_start_ip_address_array[array_count] = atoi(token);

	while(token!=NULL)
	{
		token = strtok(NULL, delim);

		if(token == NULL)
		{
			break;
		}

		else
		{
			array_count++;
			//printf("%s ", token);
			rull_dest_start_ip_address_array[array_count] = atoi(token); //정수형을 저장.//
		}
	}

	array_count = 0; //다음 ip주소대역을 저장하기 위해서 카운터를 초기화.//

	/*for(i=0; i<4; i++)
	{
		printf("%d ", rull_source_start_ip_address_array[i]);
	}*/

	printf("\n");

	token = strtok(rull_dest_end_ip, delim);

	rull_dest_end_ip_address_array[array_count] = atoi(token);

	while(token != NULL)
	{
		token = strtok(NULL, delim);

		if(token == NULL)
		{
			break;
		}

		else
		{
			array_count++;
			//printf("%s ", token);
			rull_dest_end_ip_address_array[array_count] = atoi(token); //정수형을 저장.//
		}
	}

	/*for(i=0; i<4; i++)
	{
		printf("%d ", rull_source_end_ip_address_array[i]);
	}*/

	//printf("\n");

	array_count = 0; //다음 ip주소대역을 저장하기 위해서 카운터를 초기화.//

	token = strtok(input_dest_ip, delim);

	input_dest_ip_address_array[array_count] = atoi(token);

	while(token != NULL)
	{
		token = strtok(NULL, delim);

		if(token == NULL)
		{
			break;
		}

		else
		{
			array_count++;
			//printf("%s ", token);
			input_dest_ip_address_array[array_count] = atoi(token); //정수형을 저장.//
		}
	}

	/*for(i=0; i<4; i++)
	{
		printf("%d ", input_source_ip_address_array[i]);
	}*/

	//비교할 3개의 IP주소대역을 나누었으므로 비교/판단 시작.//
	//현재 들어온 IP주소가 설정한 대역에 ACCEPT옵션이면 1을 반환, DROP옵션에 포함되면 -1을 반환.//
	//비교방법은 기준 ip(input ip)로 양 옆에 min과 max의 주소는 start, end의 IP주소로 한다.//
	//우선은 DROP모드인지 ACCEPT모드인지 구분.//

	//기본적으로 안되는 IP대역(256.0.0.0 ~)은 걸러낸다//
	if(input_dest_ip_address_array[0] >= ERROR_IP_EXCEPTION) //기본적으로 IP대역이 256.이 넘으면 에러처리.//
	{
		check_value = -1;

		return check_value; //DROP의 경우랑 같으므로 -1을 반환.//
	}

	//ACCEPT, DROP에 유무에 따른 구분.반환 시 인터페이스의 유지가 필요.//
	if(rull->d_ip_accept_drop_check == 1) //ACCEPT모드. ACCEOT모드인데 DROP에 조건을 만족 시 -1을 반환.//
	{
		printf("ACCEPT Mode apply");

		for(i=0; i<4; i++)
		{
			//첫번째 IP주소 범위(대역)에 대한 범위판단.범위이기에 대역시작 IP주소, 입력주소, 대역 마지막 IP주소로 비교한다.//
			if(rull_dest_start_ip_address_array[i] <= input_dest_ip_address_array[i] 
				&& input_dest_ip_address_array[i] <= rull_dest_end_ip_address_array[i])
			{
				//계속 true의 값을 유지하고 있는다.//
				ip_check_count++;
			}
		}

		if(ip_check_count == 4) //4개가 다 만족할 경우.현재 모드가 ACCEPT이므로 이 부분은 성공으로 간다.//
		{	
			check_value = 1;
		}

		else //1개의 영역이라도 만족하지 않을 경우. 이 경우는 현재 ACCEPT으로 보았을 시 거짓이므로 -1을 반환.비정상적인것으로 간다.//
		{
			check_value = -1;
		}
	}

	else if(rull->d_ip_accept_drop_check == 0) //DROP모드.DROP이니 해당 옵션에 만족하면 -1을 반환.아니면 1을 반환해서 계속 진행.//
	{
		printf("DROP Mode apply");

		//배열 3개를 다루게 된다.//
		for(i=0; i<4; i++)
		{
			//첫번째 IP주소 범위(대역)에 대한 범위판단.범위이기에 대역시작 IP주소, 입력주소, 대역 마지막 IP주소로 비교한다.//
			if(rull_dest_start_ip_address_array[i] <= input_dest_ip_address_array[i] 
				&& input_dest_ip_address_array[i] <= rull_dest_end_ip_address_array[i])
			{
				//계속 true의 값을 유지하고 있는다.//
				ip_check_count++;
			}
		}

		if(ip_check_count == 4) //4개가 다 만족할 경우.현재 모드가 DROP이므로 이 부분은 실패(폐기)로 간다.//
		{	
			check_value = -1; 
		}

		else //1개의 영역이라도 만족하지 않을 경우. 이 경우는 현재 DROP으로 보았을 시 참이므로 1을 반환.정상적인것으로 간다.//
		{
			check_value = 1;
		}
	}

	printf("\n");

	return check_value;
}
///////////////////////////////
int Rull_Check_Source_port(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull)
{
	int input_source_port_number;
	int rull_source_port;
	int check_value; //현재 조건에 만족하면 1을 반환, 하나라도 만족하지 않으면(DROP) 바로 버려야 하기에(IP->PORT->ControlBit) -1을 반환.//
	int i;

	/*error message
	# ERROR PACKET : It is a violation of the rules specified port number.
	strcpy(error_message, "# ERROR PACKET : It is a violation of the rules specified port number.");
	*/

	//포트번호 초기화.//
	input_source_port_number = ntohs(tcph->source); //들어온 패킷에서 풀발지 포트번호 설정.//
	rull_source_port = rull->source_port; //출발지 포트번호 전달.//

	//소스포트에 대한 비교이므로 2개의 정수값을 가지고 이루어진다.//
	//우선적으로 기본적으로 말이 안되는 포트범위(65535)를 초과하는 패킷을 걸러낸다.//
	if(input_source_port_number >= ERROR_PORT_EXCEPTION)
	{
		check_value = -1; //DROP되는 방향으로 빠지게 한다.//

		return check_value; //DROP의 경우랑 같으므로 -1을 반환.//
	}

	//ACCEPT, DROP에 따른 범위.//
	if(rull->source_port_accept_drop_check == 1) //ACCEPT의 경우.//
	{
		printf("ACCEPT Mode apply\n");
		
		if(rull_source_port == input_source_port_number) //룰에 저장된 포트번호와 들어온 패킷의 포트번호가 일치할 경우.//
		{
			//ACCEPT모드이기에 포트번호가 일치하면 성공으로 간주.//
			check_value = 1;
		}

		else if(rull_source_port != input_source_port_number) //같지 않은 경우.DROP이 된다.//
		{
			check_value = -1;
		}
	}

	else if(rull->source_port_accept_drop_check == 0) //DROP모드.//
	{
		printf("DROP Mode apply\n");

		if(rull_source_port == input_source_port_number) //룰에 저장된 포트번호와 들어온 패킷의 포트번호가 일치할 경우.//
		{
			//DROP모드이기에 포트번호가 일치하면 폐기하는 것으로 간주.//
			check_value = -1;
		}

		else if(rull_source_port != input_source_port_number) //같지 않은 경우.폐기되지 않는 패킷이 된다.//
		{
			check_value = 1;
		}
	}
	
	printf("\n");

	return check_value;
}
///////////////////////////////
int Rull_Check_Dest_port(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull)
{
	int input_dest_port_number;
	int rull_dest_port;
	int check_value; //현재 조건에 만족하면 1을 반환, 하나라도 만족하지 않으면(DROP) 바로 버려야 하기에(IP->PORT->ControlBit) -1을 반환.//
	int i;

	/*error message
	# ERROR PACKET : It is a violation of the rules specified port number.
	strcpy(error_message, "# ERROR PACKET : It is a violation of the rules specified port number.");
	*/

	//포트번호 초기화.//
	input_dest_port_number = ntohs(tcph->dest); //들어온 패킷에서 풀발지 포트번호 설정.//
	rull_dest_port = rull->dest_port; //출발지 포트번호 전달.//

	//소스포트에 대한 비교이므로 2개의 정수값을 가지고 이루어진다.//
	//우선적으로 기본적으로 말이 안되는 포트범위(65535)를 초과하는 패킷을 걸러낸다.//
	if(input_dest_port_number >= ERROR_PORT_EXCEPTION)
	{
		check_value = -1; //DROP되는 방향으로 빠지게 한다.//

		return check_value; //DROP의 경우랑 같으므로 -1을 반환.//
	}

	//ACCEPT, DROP에 따른 범위.//
	if(rull->dest_port_accept_drop_check == 1) //ACCEPT의 경우.//
	{
		printf("ACCEPT Mode apply\n");
		
		if(rull_dest_port == input_dest_port_number) //룰에 저장된 포트번호와 들어온 패킷의 포트번호가 일치할 경우.//
		{
			//ACCEPT모드이기에 포트번호가 일치하면 성공으로 간주.//
			check_value = 1;
		}

		else if(rull_dest_port != input_dest_port_number) //같지 않은 경우.DROP이 된다.//
		{
			check_value = -1;
		}
	}

	else if(rull->dest_port_accept_drop_check == 0) //DROP모드.//
	{
		printf("DROP Mode apply\n");

		if(rull_dest_port == input_dest_port_number) //룰에 저장된 포트번호와 들어온 패킷의 포트번호가 일치할 경우.//
		{
			//DROP모드이기에 포트번호가 일치하면 폐기하는 것으로 간주.//
			check_value = -1;
		}

		else if(rull_dest_port != input_dest_port_number) //같지 않은 경우.폐기되지 않는 패킷이 된다.//
		{
			check_value = 1;
		}
	}
	
	printf("\n");

	return check_value;
}
////////////////////////////////
int Rull_Check_ControlBit(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull)
{
	int i;
	int check_value;
	int input_controlbit_array[6]; //입력된 컨트롤 비트 배열을 선언.//
	int check_count = 0;
	
	printf("rull Controlbit : ");

	for(i=0; i<6; i++)
	{
		printf("%d ", rull->set_controlbit_array[i]);
	}

	printf("\n");

	(tcph->urg == 1) ? (input_controlbit_array[0] = 1) : (input_controlbit_array[0] = 0);
	(tcph->ack == 1) ? (input_controlbit_array[1] = 1) : (input_controlbit_array[1] = 0);
	(tcph->psh == 1) ? (input_controlbit_array[2] = 1) : (input_controlbit_array[2] = 0);
	(tcph->rst == 1) ? (input_controlbit_array[3] = 1) : (input_controlbit_array[3] = 0);
	(tcph->syn == 1) ? (input_controlbit_array[4] = 1) : (input_controlbit_array[4] = 0);
	(tcph->fin == 1) ? (input_controlbit_array[5] = 1) : (input_controlbit_array[5] = 0);

	printf("input Controlbit : ");

	for(i=0; i<6; i++)
	{
		printf("%d ", input_controlbit_array[i]);
	}

	//ACCEPT모드와 DROP모드를 구분.//
	if(rull->set_controlbit_accept_drop_check == 1) //ACCEPT인경우.//
	{	
		printf("ACCEPT Mode");

		for(i=0; i<6; i++)
		{
			if(input_controlbit_array[i] == rull->set_controlbit_array[i])
			{
				check_count++;
			}
		}

		if(check_count == 6) //다 맞는경우.//
		{
			check_value = 1; //ACCEPT인 경우.//
		}

		else
		{
			check_value = -1; //DROP인 경우.//
		}	
	}	

	else if(rull->set_controlbit_accept_drop_check == 0) //DROP인 경우.//
	{
		printf("DROP Mode");

		for(i=0; i<6; i++)
		{
			if(input_controlbit_array[i] == rull->set_controlbit_array[i])
			{
				check_count++;
			}
		}

		if(check_count == 6) //다 맞는경우.//
		{
			check_value = -1; //DROP인 경우.//
		}

		else
		{
			check_value = 1; //ACCEPT인 경우.//
		}
	}

	return check_value;	
}
///////////////////////////////
void save_total_log_file(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull)
{
	fputs("\n", total_packet_log_file);
	fputs("-----------------------------------", total_packet_log_file);

	fputs("\n", total_packet_log_file);

	fputs("* Packet trace Access time : ", total_packet_log_file);									
				
	timer = time(NULL); //현재 시각을 초 단위로 얻기.//
	t = localtime(&timer); //초 단위의 시간을 분리하여 구조체에 넣는다.//

	fprintf(total_packet_log_file, "%d.%d.%d %d:%d:%d\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	//파일저장 부분. 이 부분에 로그파일은 전체로그파일에 들어가게 된다.//
	fprintf(total_packet_log_file,"[IP HEADER] VER: %u HL : %u Protocol : %u ", iph->version, iph->ihl, iph->protocol);
	fprintf(total_packet_log_file,"SRC IP : %s ", inet_ntoa(*(struct in_addr *)&iph->saddr)); //Source IP출력.//
	fprintf(total_packet_log_file,"DEST IP : %s \n", inet_ntoa(*(struct in_addr *)&iph->daddr)); //Destination IP출력.//

	fprintf(total_packet_log_file,"[TCP HEADER] src port: %u dest port : %u ", ntohs(tcph->source), ntohs(tcph->dest));
	
	(tcph->urg == 1) ? fprintf(total_packet_log_file,"U") : fprintf(total_packet_log_file,"-"); //Urgent가 설정되있으면 표시, 아니면 '-'표시.//
	(tcph->ack == 1) ? fprintf(total_packet_log_file,"A") : fprintf(total_packet_log_file,"-");
	(tcph->psh == 1) ? fprintf(total_packet_log_file,"P") : fprintf(total_packet_log_file,"-");
	(tcph->rst == 1) ? fprintf(total_packet_log_file,"R") : fprintf(total_packet_log_file,"-");
	(tcph->syn == 1) ? fprintf(total_packet_log_file,"S") : fprintf(total_packet_log_file,"-");
	(tcph->fin == 1) ? fprintf(total_packet_log_file,"F") : fprintf(total_packet_log_file,"-");

	fputs("\n", total_packet_log_file);
	fputs("-----------------------------------", total_packet_log_file);
}
///////////////////////////////
void save_accept_log_file(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull)
{
	//단일조건(level1)이기에 바로 결과를 해당 로그 파일에 저장.//
	fputs("-----------------------------------",correct_packet_log_file);
	fputs("\n", correct_packet_log_file);
	//DROP의 경우이므로 파일에 저장.//
	fputs("* Packet trace Access time : ", correct_packet_log_file);						

	timer = time(NULL); //현재 시각을 초 단위로 얻기.//
	t = localtime(&timer); //초 단위의 시간을 분리하여 구조체에 넣는다.//

	fprintf(correct_packet_log_file, "%d.%d.%d %d:%d:%d\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	//파일저장 부분. 이 부분에 로그파일은 전체로그파일에 들어가게 된다.//
	fprintf(correct_packet_log_file,"[IP HEADER] VER: %u HL : %u Protocol : %u ", iph->version, iph->ihl, iph->protocol);
	fprintf(correct_packet_log_file,"SRC IP : %s ", inet_ntoa(*(struct in_addr *)&iph->saddr)); //Source IP출력.//
	fprintf(correct_packet_log_file,"DEST IP : %s \n", inet_ntoa(*(struct in_addr *)&iph->daddr)); //Destination IP출력.//

	fprintf(correct_packet_log_file,"[TCP HEADER] src port: %u dest port : %u ", ntohs(tcph->source), ntohs(tcph->dest));
	
	(tcph->urg == 1) ? fprintf(correct_packet_log_file,"U") : fprintf(correct_packet_log_file,"-"); //Urgent가 설정되있으면 표시, 아니면 '-'표시.//
	(tcph->ack == 1) ? fprintf(correct_packet_log_file,"A") : fprintf(correct_packet_log_file,"-");
	(tcph->psh == 1) ? fprintf(correct_packet_log_file,"P") : fprintf(correct_packet_log_file,"-");
	(tcph->rst == 1) ? fprintf(correct_packet_log_file,"R") : fprintf(correct_packet_log_file,"-");
	(tcph->syn == 1) ? fprintf(correct_packet_log_file,"S") : fprintf(correct_packet_log_file,"-");
	(tcph->fin == 1) ? fprintf(correct_packet_log_file,"F") : fprintf(correct_packet_log_file,"-");

	fputs("\n", correct_packet_log_file);
	fputs("SUCCESS Packet", correct_packet_log_file);
	
	fputs("\n", correct_packet_log_file);
	fputs("-----------------------------------", correct_packet_log_file);
}
///////////////////////////////
void save_drop_log_file(struct iphdr *iph, struct tcphdr *tcph, struct rull_set *rull, char error_message[])
{
	//단일조건(level1)이기에 바로 결과를 해당 로그 파일에 저장.//
	fputs("-----------------------------------", incorrect_packet_log_file);
	fputs("\n", incorrect_packet_log_file);
	//DROP의 경우이므로 파일에 저장.//
	fputs("* Packet trace Access time : ", incorrect_packet_log_file);						

	timer = time(NULL); //현재 시각을 초 단위로 얻기.//
	t = localtime(&timer); //초 단위의 시간을 분리하여 구조체에 넣는다.//

	fprintf(incorrect_packet_log_file, "%d.%d.%d %d:%d:%d\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	//파일저장 부분. 이 부분에 로그파일은 전체로그파일에 들어가게 된다.//
	fprintf(incorrect_packet_log_file,"[IP HEADER] VER: %u HL : %u Protocol : %u ", iph->version, iph->ihl, iph->protocol);
	fprintf(incorrect_packet_log_file,"SRC IP : %s ", inet_ntoa(*(struct in_addr *)&iph->saddr)); //Source IP출력.//
	fprintf(incorrect_packet_log_file,"DEST IP : %s \n", inet_ntoa(*(struct in_addr *)&iph->daddr)); //Destination IP출력.//

	fprintf(incorrect_packet_log_file,"[TCP HEADER] src port: %u dest port : %u ", ntohs(tcph->source), ntohs(tcph->dest));
	
	(tcph->urg == 1) ? fprintf(incorrect_packet_log_file,"U") : fprintf(incorrect_packet_log_file,"-"); //Urgent가 설정되있으면 표시, 아니면 '-'표시.//
	(tcph->ack == 1) ? fprintf(incorrect_packet_log_file,"A") : fprintf(incorrect_packet_log_file,"-");
	(tcph->psh == 1) ? fprintf(incorrect_packet_log_file,"P") : fprintf(incorrect_packet_log_file,"-");
	(tcph->rst == 1) ? fprintf(incorrect_packet_log_file,"R") : fprintf(incorrect_packet_log_file,"-");
	(tcph->syn == 1) ? fprintf(incorrect_packet_log_file,"S") : fprintf(incorrect_packet_log_file,"-");
	(tcph->fin == 1) ? fprintf(incorrect_packet_log_file,"F") : fprintf(incorrect_packet_log_file,"-");

	fprintf(incorrect_packet_log_file, "\n\n%s", error_message);
	
	fputs("\n", incorrect_packet_log_file);
	fputs("-----------------------------------", incorrect_packet_log_file);
}
///////////////////////////////
void sig_handler(int sig_num)
{
	if(sig_num == SIGINT)
	{
		//현재 부분에서는 패킷 스캔을 종료하고 마무리 작업을 한다.//
		printf("\n");

		printf("\033[%dm\nPacket filtering exit...\033[0m\n\n", 31);

		if(correct_packet_log_file != NULL)
		{
			fclose(correct_packet_log_file);
		}

		if(incorrect_packet_log_file != NULL)
		{
			fclose(incorrect_packet_log_file);
		}

		if(total_packet_log_file != NULL)
		{
			fclose(total_packet_log_file); //파일을 닫는다.//
		}

		exit(1);
	}
}
