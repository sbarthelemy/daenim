
#include "SocketCallback"



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
SocketCallback::SocketCallback(osg::Node* node, SOCKET _s):
s(_s)
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
    memset(msg, 0x0, MAX_LEN);
#if defined WIN32
    int numRead = recv(s, msg, MAX_LEN, 0); //MSG_WAITALL
#elif defined UNIX
    int numRead = recv(s, msg, MAX_LEN, MSG_DONTWAIT);
#endif
    
    if (numRead>=0) {
        //printf("Message size: %i\nMessage: %s;\n\n", numRead, msg);
        //std::cout<<"Message size: "<<numRead<<std::endl<<msg<<std::endl<<std::endl;
        if (strncmp(msg, "close connection", 16) == 0)
        {
            shutdown(s,2);
            std::cout<<"SocketCallBack is shutted down"<<std::endl;
        }
        else {
            char * pch;
            pch = strtok(msg, "\n");
            while (pch !=NULL) {
                char name[128];
                float f00, f01, f02, f03, f10, f11, f12, f13, f20, f21, f22, f23;
                sscanf(pch, "%s %f %f %f %f %f %f %f %f %f %f %f %f", name, &f00, &f01, &f02 ,&f03, &f10, &f11, &f12, &f13, &f20, &f21, &f22, &f23);
                pch = strtok(NULL, "\n");
                
                if (validNode.find(name) !=validNode.end()) {
                    osg::Matrix m(f00,f10,f20,0,
                                  f01,f11,f21,0,
                                  f02,f12,f22,0,
                                  f03,f13,f23,1);
                    validNode[name]->setMatrix(m);
                }
            }
        }
    }
    traverse(node, nv); 
}


