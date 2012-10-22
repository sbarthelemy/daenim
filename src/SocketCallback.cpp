
#include "SocketCallback"

//#include <iostream>
#include <sstream>

SOCKET OpenPort(const char* _host, const int _port) {

    SOCKET sock;
    SOCKADDR_IN sin;

#if defined WIN32
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2,0), &WSAData);

    sin.sin_addr.s_addr = inet_addr(_host);
    sin.sin_family      = AF_INET;
    sin.sin_port        = htons(_port);

#elif defined UNIX
    struct hostent *server;
    server = gethostbyname(_host);

    bzero(&sin, sizeof(sin));
    sin.sin_family      = AF_INET;
    sin.sin_port        = htons(_port);

#endif

    sock = socket(AF_INET,SOCK_STREAM,0);

    if (connect(sock, (SOCKADDR *)&sin, sizeof(sin)) != 0) {
        printf("Connection problem!!!\n");
    };

#if defined WIN32
    u_long nonBlockingMode = 1;
    ioctlsocket(sock, FIONBIO, &nonBlockingMode);
#endif

    return sock;
}


void ClosePort(SOCKET sock) {
#if defined WIN32
    closesocket(sock);
    WSACleanup();
#endif
}





//#define MAX_LEN 2048
#define MAX_LEN 4096
SocketCallback::SocketCallback(osg::Node* node, SOCKET _s)
    :s(_s)
    ,communication_is_running(true)
{
    parse(node);
}

SocketCallback::~SocketCallback()
{
    shutdown(s,2);
    ClosePort(s);
}

void SocketCallback::parse(osg::Node* curNode)
{
    if (curNode->asTransform()) {
        osg::MatrixTransform* mat = curNode->asTransform()->asMatrixTransform();
        if (mat) {
            validNode[mat->getName()] = mat;
        }
    }
    osg::Group* curGroup = curNode->asGroup();
    if (curGroup) {
        for (unsigned int i = 0 ; i < curGroup->getNumChildren(); i ++) {
            parse(curGroup->getChild(i));
        }
    }
}



void SocketCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    char msg[MAX_LEN];
    std::string complete_msg;

    int numRead;

    if (communication_is_running)
    {
        memset(msg, 0x0, MAX_LEN);
#if defined WIN32
        numRead = recv(s, msg, MAX_LEN, 0);
#elif defined UNIX
        numRead = recv(s, msg, MAX_LEN, MSG_DONTWAIT);
#endif
        complete_msg.append(msg, 0, numRead);
        
        if (numRead>0)
        {
            while (numRead == MAX_LEN)
            {
                memset(msg, 0x0, MAX_LEN);
#if defined WIN32
                numRead = recv(s, msg, MAX_LEN, 0);
#elif defined UNIX
                numRead = recv(s, msg, MAX_LEN, MSG_DONTWAIT);
#endif
                complete_msg.append(msg, numRead);
            }
            
            // The complete message is received, we can treat it now
            std::string name;
            float data[12];
            std::stringstream ss2(complete_msg);
            
            while(ss2 >> name)
            {
                if (name == "close_connection")
                {   communication_is_running=false;
                    shutdown(s,2);
                    std::cout<<"SocketCallBack is shutted down"<<std::endl;
                    break;
                }
                
                ss2 >>data[0]>>data[1]>>data[2]>>data[3]
                    >>data[4]>>data[5]>>data[6]>>data[7]
                    >>data[8]>>data[9]>>data[10]>>data[11];
                
                if (validNode.find(name.c_str()) != validNode.end())
                {
                            osg::Matrix m(data[0],data[4],data[8] ,0,
                                          data[1],data[5],data[9] ,0,
                                          data[2],data[6],data[10],0,
                                          data[3],data[7],data[11],1);
                            validNode[name]->setMatrix(m);
                }
            }
        }
    }
    traverse(node, nv); 
}

/*
void SocketCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    char msg[sizeof(ArbTransform)];
    memset(msg, 0x0, sizeof(ArbTransform));
    ArbTransform cAt;

    for (int i=0; i<100000; i++) { // big "for" loop to avoid infinite loop
#if defined WIN32
        int numRead = recv(s, msg, sizeof(ArbTransform), 0); //MSG_WAITALL
#elif defined UNIX
        int numRead = recv(s, msg, sizeof(ArbTransform), MSG_DONTWAIT);
#endif
        if (numRead<=0) {
            break;
        }
        else {
            //printf("Message size: %i\nMessage: %s;\n\n", numRead, msg);
            //std::cout<<"Message size: "<<numRead<<std::endl<<msg<<std::endl<<std::endl;
            if (strncmp(msg, "close connection", 16) == 0)
            {
                shutdown(s,2);
                std::cout<<"SocketCallBack is shutted down"<<std::endl;
            }
            else if (strncmp(msg, "update done", 11) == 0)
            {
                send(s, "OK", 2, 0);
                break;
            }
            else {
                memcpy(&cAt, msg, sizeof(ArbTransform));
                if (validNode.find(cAt.name) != validNode.end()) {
                    osg::Matrix m(cAt.val[0],cAt.val[4],cAt.val[8] ,0,
                                  cAt.val[1],cAt.val[5],cAt.val[9] ,0,
                                  cAt.val[2],cAt.val[6],cAt.val[10],0,
                                  cAt.val[3],cAt.val[7],cAt.val[11],1);
                    validNode[cAt.name]->setMatrix(m);
                }
            }
        }
    }
    traverse(node, nv); 
}
*/


