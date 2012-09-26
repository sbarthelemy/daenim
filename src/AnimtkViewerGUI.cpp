/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 *
 * Authors:
 * Cedric Pinson <mornifle@plopbyte.net>
 * jeremy Moles <jeremy@emperorlinux.com>
*/

#include "AnimtkViewerGUI"

#include <stdio.h>

#include <osgViewer/ViewerEventHandlers>
#include <osg/DeleteHandler>

//-------------- Some definition to set the viewer interface -----------------//
#define BUTTON_HEIGHT     64.0f
#define CURSOR_SIZE     5.0f
#define SLIDER_HEIGHT   25.0f
#define SPEED_WIDTH     100.0f
#define SPEED_HEIGHT    25.0f
#define FONT_SIZE       25.0f


#if defined WIN32
    #include <windows.h>
    #define APP_PATH_FINDER GetModuleFileName
    #define FIRST_ARG NULL
    #define ICON_PATH "Icons\\"
    #define FONT "FreeMono.ttf"
#elif defined APPLE
    #include <mach-o/dyld.h>
    #define APP_PATH_FINDER _NSGetExecutablePath
    #define ICON_PATH "Icons/"
    #define FONT "FreeMono.ttf"
    #include <sys/stat.h>
    #include <sys/types.h>
#elif defined UNIX
    #include <unistd.h>
    #define APP_PATH_FINDER readlink
    #define FIRST_ARG "/proc/self/exe"
    #define ICON_PATH "../share/daenim/Icons/"
    #define FONT "/usr/share/fonts/truetype/freefont/FreeMono.ttf"
    #include <sys/stat.h>
    #include <sys/types.h>
#endif


std::string getIconAbsolutePath()
{
    char result[2048];
#if defined APPLE
    uint32_t res_size = sizeof(result);
    APP_PATH_FINDER(result, &res_size);
#else
    APP_PATH_FINDER( FIRST_ARG, result, 2048 );
#endif
    std::string str_result;
    str_result = result;
    size_t found = str_result.find_last_of("/\\");
    return str_result.substr(0,found+1) + ICON_PATH;
}

const std::string IMAGE_PATH = getIconAbsolutePath();


#define SPEED_SLOTS     13
float possibleSpeed[SPEED_SLOTS] = {0.0f, 0.05f, 0.1f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 10.0f};



// This is a temporary hack to "prevent" dragging on Widgets and Windows. TODO
bool eatDrag(osgWidget::Event&) 
{
    return true;
}


AnimtkViewerGUI::AnimtkViewerGUI(osgViewer::Viewer* ve, unsigned int w, unsigned int h, 
                                 osgAnimation::BasicAnimationManager* basicAnimManager, std::string extension):
osgWidget::WindowManager(ve, w, h, 0x80000000, 0)
{
    viewer = (osgViewer::ViewerExt*)ve;
    _extension = extension;
    _basicAnimManager = basicAnimManager;
    
    if (_basicAnimManager)
    {
        setAnimationManager();

        _createInfoBar();
        _createButtonBoundingBox();
        _createSliderBoundingBox();
        _createSpeedBoundingBox();
        

        // Remember, you can't call resizePercent until AFTER the box is parented
        // by a WindowManager; how could it possibly resize itself if it doesn't know
        // how large it's viewable area is?
        _infoLabelBox->resizePercent(100.0f);
        _sliderBarBox->resizePercent(100.0f);
        _buttonBox->resizePercent(100.0f);
        
        
    }

}

void AnimtkViewerGUI::setAnimationManager()
{
    std::cout<<"WARNING: we only select the first animation of the AnimationList"<<std::endl;
    if (_basicAnimManager)
    {
        animation = _basicAnimManager->getAnimationList()[0];
        animDuration = animation->getDuration();
        std::cout<<"Animation: "<<animation->getName()<<" saved"<<std::endl;

        viewer->setTimelineFromAnimation(animation);
        _basicAnimManager->playAnimation(animation);
    }
}


void AnimtkViewerGUI::accept(osg::NodeVisitor &nv)
{
    if (_basicAnimManager)
    {
        setSpeedSlot(getSpeedSlot());
        
        // Update frame number...
        char buffer[256];
        sprintf(buffer, "frame %u", viewer->getFrame());
        _frameLabel->setLabel(buffer);

        _infoLabelBox->resizePercent(100.0f);
        _sliderBarBox->resizePercent(100.0f);
        _buttonBox->resizePercent(100.0f);
    }
    WindowManager::accept(nv);
}


osgWidget::Widget* AnimtkViewerGUI::_createButton(const std::string& name)
{
    osgWidget::Widget* b = new osgWidget::Widget(name, BUTTON_HEIGHT, BUTTON_HEIGHT);
    if(!b) return 0;

    b->setImage(IMAGE_PATH + name + ".png", true);
    b->setEventMask(osgWidget::EVENT_MASK_MOUSE_DRAG);

    ButtonFunctor* bt = new ButtonFunctor();
    b->setUpdateCallback(bt);

    b->addCallback(new osgWidget::Callback(&ButtonFunctor::enter, bt, osgWidget::EVENT_MOUSE_ENTER));
    b->addCallback(new osgWidget::Callback(&ButtonFunctor::leave, bt, osgWidget::EVENT_MOUSE_LEAVE));
    b->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_buttonPush, this, osgWidget::EVENT_MOUSE_PUSH));
    b->addCallback(new osgWidget::Callback(&eatDrag, osgWidget::EVENT_MOUSE_DRAG));

    return b;
}

bool AnimtkViewerGUI::_buttonPush(osgWidget::Event& ev) 
{
    if(!ev.getWidget()) return false;

    // We're safe at this point, so begin processing.
    
    std::string name = ev.getWidget()->getName();

    if(name == "play")
    {
        setPause(!viewer->getPause());
    }
    else if(name == "stop") {
        setPause(true);
        viewer->setFrame(0);
        viewer->setSpeed(1.0f);
    }
    else if(name == "rap")
    {
        setSpeedSlot(getSpeedSlot()+1);
    }
    else if(name == "ral")
    {
        setSpeedSlot(getSpeedSlot()-1);
    }
    else if(name == "next")
    {
        setPause(true);
        viewer->setFrame(viewer->getFrame()+1);
    }
    else if(name == "back")
    {
        setPause(true);
        viewer->setFrame(viewer->getFrame()-1);
    }
    else if(name == "rec")
    {
        startRecordAnimation();
    }
    else
    {
        return false;
    }

    return true;
}



void AnimtkViewerGUI::_createButtonBoundingBox() 
{
    //--- Create the Bounding Box where will be located the buttons ---//
    _buttonBox = new osgWidget::Box("buttonBox", osgWidget::Box::HORIZONTAL); //HORIZONTAL ==> create a bounding box which expands along the horizontal
    _buttonBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.7f);
    _buttonBox->addCallback(new osgWidget::Callback(&eatDrag, osgWidget::EVENT_MOUSE_DRAG));
    

    //--- Create a space ==> 2 spaces on each side will center the buttons in the bounding box
    osgWidget::Widget* space = new osgWidget::Widget("nullSpace", 0.0f, 0.0f);
    space->setCanFill(true);                    //the nullspace will expand to fill all the space (we set horizontal above)
    space->setColor(0.0f, 0.0f, 0.0f, 0.0f);    //we dont want to see this null space. else we get blank space

    //-------------- Create all the buttons (in order of apparition) -----------------//
    _buttonBox->addWidget(space);
    _buttonBox->addWidget( _createButton("ral")  );
    _buttonBox->addWidget( _createButton("rap")  );
    _buttonBox->addWidget( _createButton("play") ); // will do pause too!
    _buttonBox->addWidget( _createButton("back") );
    _buttonBox->addWidget( _createButton("next") );
    _buttonBox->addWidget( _createButton("stop") );
    _buttonBox->addWidget( _createButton("rec") );
    _buttonBox->addWidget(osg::clone(space, "space1", osg::CopyOp::DEEP_COPY_ALL));

    addChild(_buttonBox.get());
}

void AnimtkViewerGUI::_createSliderBoundingBox()
{
    //------------ Create the BackGround Bar with eventHandler ----------//
    _sliderBarBox = new osgWidget::Box("sliderBarBox", osgWidget::Box::HORIZONTAL);
    _sliderBarBox->getBackground()->setColor(0.0f, 1.0f, 0.0f, 0.5f);
    _sliderBarBox->setOrigin(0.0f, BUTTON_HEIGHT);

    osgWidget::NullWidget* _sliderBarNull = new osgWidget::NullWidget("sliderBarNull", 0.0f, SLIDER_HEIGHT);
    _sliderBarNull->setColor(0.0f, 0.0f, 0.9f, 0.5f);
    _sliderBarNull->setCanFill(true);
    _sliderBarNull->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_sliderDown, this, osgWidget::EVENT_MOUSE_PUSH));
    _sliderBarNull->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_sliderDrag, this, osgWidget::EVENT_MOUSE_DRAG));
    _sliderBarBox->addWidget(_sliderBarNull);
    
    //------------ Create the Associate Cursor ----------//
    _sliderCursorBox = new osgWidget::Box("sliderCursorBox", osgWidget::Box::HORIZONTAL);
    _sliderCursorBox->setOrigin(0.0f, BUTTON_HEIGHT);

    osgWidget::NullWidget* _sliderCursorNull = new osgWidget::NullWidget("sliderCursorNull", CURSOR_SIZE, SLIDER_HEIGHT);
    _sliderCursorNull->setColor(0.9f, 0.9f, 0.9f, 1.0f);
    _sliderCursorBox->addWidget(_sliderCursorNull);

    //--------------- Init and register ----------------//
    _sliderPos = _sliderPercent = 0.0f;
    addChild(_sliderCursorBox.get());
    addChild(_sliderBarBox.get());
}

void AnimtkViewerGUI::_createSpeedBoundingBox()
{
    //------------ Create the BackGround Bar with eventHandler ----------//
    _speedBarBox = new osgWidget::Box("speedBox", osgWidget::Box::HORIZONTAL);
    _speedBarBox->getBackground()->setColor(0.0f, 1.0f, 0.0f, 0.5f);
    _speedBarBox->setOrigin(0.0f, BUTTON_HEIGHT + SLIDER_HEIGHT);
    
    osgWidget::NullWidget* speedBarNull = new osgWidget::NullWidget("speedBarNull", SPEED_WIDTH, SPEED_HEIGHT);
    speedBarNull->setColor(0.9f, 0.0f, 0.0f, 0.5f);
    speedBarNull->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_speedDown, this, osgWidget::EVENT_MOUSE_PUSH));
    speedBarNull->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_speedDrag, this, osgWidget::EVENT_MOUSE_DRAG));
    _speedBarBox->addWidget( speedBarNull );

    //------------ Create the Associate Cursor ----------//
    _speedCursorBox = new osgWidget::Box("speedCursorBox", osgWidget::Box::HORIZONTAL);
    _speedCursorBox->setOrigin(0.0f, BUTTON_HEIGHT + SLIDER_HEIGHT);
    
    osgWidget::NullWidget* speedCursorNull = new osgWidget::NullWidget("speedCursorNull", CURSOR_SIZE, SPEED_HEIGHT);
    speedCursorNull->setColor(0.9f, 0.9f, 0.9f, 1.0f);
    _speedCursorBox->addWidget( speedCursorNull );
    
    //--------------- Register --------------------//
    addChild(_speedCursorBox.get());
    addChild(_speedBarBox.get());
}

void AnimtkViewerGUI::_createInfoBar()
{
    _infoLabelBox = new osgWidget::Box("infoLabelBox", osgWidget::Box::HORIZONTAL);
    _infoLabelBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);
    _infoLabelBox->setOrigin(SPEED_WIDTH, BUTTON_HEIGHT + SLIDER_HEIGHT); 

    osgWidget::Widget* labelSpeedSpace = new osgWidget::Widget("labelSpeedSpace", SPEED_WIDTH, 0.0f);
    labelSpeedSpace->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    _speedLabel = new osgWidget::Label("speedlabel");
    _speedLabel->setFont(FONT);
    _speedLabel->setCanFill(true);
    _speedLabel->setFontSize(FONT_SIZE);
    _speedLabel->setAlignHorizontal(osgWidget::Widget::HA_CENTER);
    
    _timeLabel = osg::clone(_speedLabel.get(), "timeLabel",osg::CopyOp::DEEP_COPY_ALL);
    
    _frameLabel = osg::clone(_speedLabel.get(), "frameLabel",osg::CopyOp::DEEP_COPY_ALL);
    
    //---------- Create all the widgets (in order of apparition) -------------//
    _infoLabelBox->addWidget(_speedLabel);
    _infoLabelBox->addWidget(_timeLabel);
    _infoLabelBox->addWidget(_frameLabel);
    _infoLabelBox->addWidget( labelSpeedSpace ); //add a null space outside the view to set well all the label in the viewer

    addChild( _infoLabelBox.get() );
}






bool AnimtkViewerGUI::_speedDown(osgWidget::Event& ev) 
{
    _speedPos = ev.x;
    _setSpeedPos(ev.x);
    return true;
}

bool AnimtkViewerGUI::_speedDrag(osgWidget::Event& ev) 
{
    _speedPos += ev.x;
    _setSpeedPos(_speedPos);
    return true;
}


void AnimtkViewerGUI::_setSpeedPos(float pos)
{
    if (pos < 0) pos = 0;
    else if (pos >= _speedBarBox->getWidth()) pos = _speedBarBox->getWidth() - 1;

    _speedPercent = pos / (_speedBarBox->getWidth() - 1);

    // Convertir le pourcentage en entier de 0 à 12
    int slot = (int)(_speedPercent * (SPEED_SLOTS-1) + 0.5f);
    if (slot < 0) slot = 0;
    if (slot >= SPEED_SLOTS) slot = SPEED_SLOTS - 1;

    // Reconvertir le slot en pourcentage...
    _speedPercent = slot/(float)(SPEED_SLOTS-1);

    _speedCursorBox->setX(_speedPercent*(_speedBarBox->getWidth()-5));
    resizeAllWindows();

    viewer->setSpeed(possibleSpeed[slot]);
    
    //------------------- Update Speed Label --------------------//
    char buffer[256];
    sprintf(buffer, "x %.1f", possibleSpeed[slot]);
    _speedLabel->setLabel(buffer);
}

int AnimtkViewerGUI::getSpeedSlot()
{
    _setSliderTime(viewer->getCurrentTime());

    int i;
    for (i=0; i<SPEED_SLOTS-1; i++)
        if (viewer->getSpeed() < (possibleSpeed[i] + possibleSpeed[i+1])/2)
            break;
    return i;
}

void AnimtkViewerGUI::setSpeedSlot(int i)
{
    _setSpeedPos( (i/(float)(SPEED_SLOTS-1))*(_speedBarBox->getWidth()-1) );
}





bool AnimtkViewerGUI::_sliderDown(osgWidget::Event& ev) 
{
    setPause(true); // Pause while moving the slider

    _setSliderPos(ev.x);
    return true;
}

bool AnimtkViewerGUI::_sliderDrag(osgWidget::Event& ev) 
{
    _setSliderPos(_sliderPos+ev.x);
    return true;
}

void AnimtkViewerGUI::_setSliderPos(float pos)
{
    _sliderPos = pos;

    _sliderPercent = pos / (getWidth());
    _sliderCursorBox->setX(_sliderPercent*(getWidth()));

    viewer->setCurrentTime(animDuration*_sliderPercent);
}

void AnimtkViewerGUI::_setSliderTime(float time)
{
    float pos = (time / animDuration) * getWidth();
    _setSliderPos(pos);
    char buffer[256];
    sprintf(buffer, "%02d:%02d.%03d", (int)(time/60), ((int)time)%60, ((int)(time*1000))%1000);
    _timeLabel->setLabel(buffer);
}

void AnimtkViewerGUI::setPause(bool state)
{
    viewer->setPause(state);
    if (state)
    {
        _buttonBox->getByName("play")->setImage(IMAGE_PATH + "play.png", true);
    }
    else
    {
        _buttonBox->getByName("play")->setImage(IMAGE_PATH + "pause.png", true);
    }
}


void AnimtkViewerGUI::startRecordAnimation()
{
    char buffer[256];
    std::string recordDirectoryName("daenim_recordAnimation");


    setPause(true);
    osg::Node::NodeMask mask = viewer->getCamera()->getCullMask();
    mask &= ~(1 << 31);
    viewer->getCamera()->setCullMask(mask);


    osg::ref_ptr<osgViewer::ScreenCaptureHandler> snapshotHandler = new osgViewer::ScreenCaptureHandler();
    osg::ref_ptr<WriteInFile> writerOperator  = new WriteInFile("png");
    snapshotHandler->setCaptureOperation(writerOperator);


#if defined WIN32
        CreateDirectory(recordDirectoryName.c_str(), NULL);
#elif defined UNIX
        mkdir(recordDirectoryName.c_str(), 0755);
#endif

    writerOperator->setFileName("000000.png");
    snapshotHandler->captureNextFrame(*viewer);
    viewer->setFrame(0);
    viewer->frame(viewer->getCurrentTime());
    for (int i=0; i<viewer->getTotalFrame(); i++)
    {
        std::cout<<i<<std::endl;
#if defined WIN32
        sprintf(buffer, ".\\%s\\%06i.png", recordDirectoryName.c_str(), i);
#elif defined UNIX
        sprintf(buffer, "./%s/%06i.png", recordDirectoryName.c_str(), i);
#endif
        writerOperator->setFileName(std::string(buffer));
        snapshotHandler->captureNextFrame(*viewer);
        viewer->setFrame(i);
        viewer->frame(viewer->getCurrentTime());
    }

    snapshotHandler->setCaptureOperation(NULL);
    
    
    
    mask |= (1 << 31);
    viewer->getCamera()->setCullMask(mask);

    viewer->setFrame(0);
}



/*=======================================
 * ButtonFunctor
 *=====================================*/

ButtonFunctor::ButtonFunctor():
_speed(5)
{
    _direction = -_speed;
    _previous = osg::Timer::instance()->tick();
}

bool ButtonFunctor::enter(osgWidget::Event& e)
{
    _direction = _speed; 
    return true;
}

bool ButtonFunctor::leave(osgWidget::Event& e)
{
    _direction = -_speed; 
    return true;
}

void ButtonFunctor::update(osg::NodeVisitor* nv , osg::Drawable* geom) 
{
    osg::Timer_t currentTick = osg::Timer::instance()->tick();
    float dt =  osg::Timer::instance()->delta_s(_previous, currentTick);
    _previous = currentTick;
    update(dt,dynamic_cast<osgWidget::Widget*>(geom));
}

void ButtonFunctor::update(float t, osgWidget::Widget* w)
{
    if (!w) return;
    _motion.update(t*_direction); 
    float val = _motion.getValue()*0.5;
    val += 0.5;
    if (val >= 1.0) val = 1.0;
    w->setColor(osg::Vec4(val, val, val, 1));
}





/*=======================================
 * WriteInFile function
 *=====================================*/

WriteInFile::WriteInFile(const std::string extension):
_filename("000000"),
_extension(extension)
{
}

void WriteInFile::operator () (const osg::Image& image, const unsigned int context_id)
{
    osgDB::writeImageFile(image, _filename);
    std::cout<<"ScreenCaptureHandler: Taking screenshot '"<<_filename<<"'"<<std::endl;
}

void WriteInFile::setFileName(std::string filename)
{
    _filename = filename;
};

