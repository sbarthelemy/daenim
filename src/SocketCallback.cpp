
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

    return sock;
}


void ClosePort(SOCKET sock) {
#if defined WIN32
    closesocket(sock);
    WSACleanup();
#endif
}






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
    char msg[N];
    memset(msg, 0x0, N);
#if defined WIN32
    int numRead = recv(s, msg, N, 0); //MSG_WAITALL
#elif defined UNIX
    int numRead = recv(s, msg, N, MSG_DONTWAIT);
#endif
    
    if (numRead>=0) {
        //printf("Message size: %i\nMessage: %s;\n\n", numRead, msg);
        if (strncmp(msg, "close connection", 16) == 0) {
            shutdown(s,2);
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



/*
int main(int argc, char** argv) {
    
    char host[128] = "127.0.0.1";
    int port=5554;
    char daeFile[256] = "/home/joe/shapes.dae";
    
    int idx;
    osg::ArgumentParser arg(&argc, argv);
    
    idx = arg.find("-host");
    if (idx>=0) {
        strcpy(host, arg[idx+1]);
    }
    idx = arg.find("-port");
    if (idx>=0) {
        port = atoi(arg[idx+1]);
    }
    idx = arg.find("-file");
    if (idx>=0) {
        strcpy(daeFile, arg[idx+1]);
    }


    SOCKET sock = OpenPort(host, port);

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(600,100,640,480);

    osg::Node* node = osgDB::readNodeFile(daeFile);
    node->setUpdateCallback(new MyCallback(node, sock));

    viewer.setSceneData(node);
    viewer.run();
    
	ClosePort(sock);

    return 0;
}
*/



