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

#include "AnimtkViewer"
#include "AnimtkViewerGUI"
#include "ViewerExt"

#include <osg/Version>
#include <osgWidget/WindowManager>
#include <osgAnimation/EaseMotion>

#define SPEED_SLOTS	13
#define FONT_COLOR	1.0f, 1.0f, 1.0f, 1.0f

#ifndef INSTALL_PREFIX
#define INSTALL_PREFIX "/usr/local"
#endif

const std::string IMAGE_PATH = std::string(INSTALL_PREFIX) + "/share/daenim/Icons/";

template <class T>
struct Sampler: public osg::Drawable::UpdateCallback 
{
    T _motion;
    Sampler() {
    }
};

typedef Sampler<osgAnimation::OutQuadMotion> WidgetSampler;

struct ButtonFunctor: public WidgetSampler 
{
    float _direction;
    float _previous;

    const float _speed;
	
    ButtonFunctor(): _speed(5) { _direction = -_speed; _previous = osg::Timer::instance()->tick();}

    bool enter(osgWidget::Event&)
    {
        _direction = _speed; 
        return true;
    }

    bool leave(osgWidget::Event&)
    {
        _direction = -_speed; 
        return true;
    }

	
    void update(osg::NodeVisitor* nv , osg::Drawable* geom) 
    {
		osg::Timer_t currentTick = osg::Timer::instance()->tick();
		
        float dt =  osg::Timer::instance()->delta_s(_previous, currentTick);
        _previous = currentTick;
        update(dt,dynamic_cast<osgWidget::Widget*>(geom));
    }

    void update(float t, osgWidget::Widget* w)
    {
        if (!w) return;
        _motion.update(t*_direction); 
        float val = _motion.getValue()*0.5;
        val += 0.5;
        if (val >= 1.0)
            val = 1.0;
        w->setColor(osg::Vec4(val, val, val, 1));
    }
};

struct LabelFunctor: public WidgetSampler 
{
    float _previous;
    bool  _active;

    const float _fadeOutTime;

    osgAnimation::OutCubicMotion _scaleSampler;

    LabelFunctor():
        _fadeOutTime(1.5f) 
{
        _previous = 0.0f;
        _active   = false;

        _scaleSampler = osgAnimation::OutCubicMotion(0.5, 1.0, 1.0);
    }

    void setActive(bool active) 
{
        _active = active;

        if(active) _motion.reset();

        _scaleSampler.reset();
    }

    void update(osg::NodeVisitor* nv , osg::Drawable* geom) 
{
		osg::Timer_t currentTick = osg::Timer::instance()->tick();
		
		float dt =  osg::Timer::instance()->delta_s(_previous, currentTick);
		_previous = currentTick;

        if(!_active) return;

        update(dt, dynamic_cast<osgWidget::Label*>(geom));
        updateScale(dt, dynamic_cast<osgWidget::Label*>(geom));
    }

    void update(float t, osgWidget::Label* w) 
{
        if(!w) return;

        _motion.update(t / _fadeOutTime);

        float val = _motion.getValue();

        if(val >= 1.0f) {
            _motion.reset();
            _active = false;
        }

        w->setFontColor(osg::Vec4(0.0f, 0.0f, 0.0f, (1.0f - val) * 0.7f));
    }

    void updateScale(float t, osgWidget::Label* w) 
{
        _scaleSampler.update(t);
        float val = _scaleSampler.getValue();
        osgWidget::Window* win = w->getParent();
        win->setScale(val);
        win->update();
    }

};


struct ListFunctor: public osg::NodeCallback 
{
    float _previous;
    int   _direction;

    osgAnimation::InQuadMotion _transformSampler;

    ListFunctor() 
{
        _direction = 1;
        _previous  = 0;

        _transformSampler.update(1.0f);
    }

    void toggleShown() 
{
        if(_direction == 1) _direction = -1;

        else _direction = 1;
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) 
{
		osg::Timer_t currentTick = osg::Timer::instance()->tick();
		
		float dt =  osg::Timer::instance()->delta_s(_previous, currentTick);
		_previous = currentTick;

        _transformSampler.update((dt * _direction) / 0.5f);

        float val = _transformSampler.getValue();

        if(val > 1.0f || val < 0.0f) return;

        osgWidget::Window* win = dynamic_cast<osgWidget::Window*>(node);

        float w   = win->getWidth();
        float wmw = win->getWindowManager()->getWidth();

        win->setX((wmw - w) + (val * w));
        win->update();
    }

};

// This is a temporary hack to "prevent" dragging on Widgets and Windows.
bool eatDrag(osgWidget::Event&) 
{
    return true;
}

AnimtkViewerGUI::AnimtkViewerGUI(osgViewer::View* view, float w, float h, unsigned int mask, bool hasAnim):
    osgWidget::WindowManager(view, w, h, mask, 0) 
{
	_hasAnim = hasAnim;
	if (hasAnim)
	{
		_createSpeedBox();
		_createSlider();
		_createButtonBox();
		
		_createLabelBox();
		_createListBox();

		_labelBox->setAnchorHorizontal(osgWidget::Window::HA_LEFT);
		_labelBox->setY(74.0f);
		_labelBox->setVisibilityMode(osgWidget::Window::VM_ENTIRE);

		_listBox->setOrigin(getWidth(), 74.0f);
		_sliderBox->setOrigin(0.0f, 74.0f);
		_sliderFrontBox->setOrigin(0.0f, 74.0f);
		_speedBox->setOrigin(0.0f, 99.0f);
		_speedFrontBox->setOrigin(0.0f, 99.0f);
		
		_speedLabelBox->setVisibilityMode(osgWidget::Window::VM_ENTIRE);
		_speedLabelBox->setOrigin(124.0f, 102.0f);
		
		_timeLabelBox->setVisibilityMode(osgWidget::Window::VM_ENTIRE);
		_timeLabelBox->setOrigin(224.0f, 102.0f);
		
		_frameLabelBox->setVisibilityMode(osgWidget::Window::VM_ENTIRE);
		_frameLabelBox->setOrigin(324.0f, 102.0f);
		
		addChild(_sliderFrontBox.get());
		addChild(_speedLabelBox.get());
		addChild(_timeLabelBox.get());
		addChild(_frameLabelBox.get());
		addChild(_sliderBox.get());
		addChild(_speedFrontBox.get());
		addChild(_speedBox.get());
		addChild(_buttonBox.get());
		addChild(_labelBox.get());
		addChild(_listBox.get());

		resizeAllWindows();

		// Remember, you can't call resizePercent until AFTER the box is parented
		// by a WindowManager; how could it possibly resize itself if it doesn't know
		// how large it's viewable area is?
		_sliderBox->resizePercent(100.0f);
		_buttonBox->resizePercent(100.0f);
		_buttonBox->resizeAdd(0.0f, 10.0f);
	}

}





void AnimtkViewerGUI::_createSpeedBox() {
	float height = 25.0f;
    _speedBox = new osgWidget::Box("speedBox", osgWidget::Box::HORIZONTAL);
	_speedFrontBox = new osgWidget::Box("speedFrontBox", osgWidget::Box::HORIZONTAL);
	
	_speedLeft= new osgWidget::NullWidget("_speedLeft", 100.0f, height);
    _speedLeft->setColor(0.9f, 0.0f, 0.0f, 0.5f);
	_speedLeft->setCanFill(true);
	
	
	_speedCenter= new osgWidget::NullWidget("_speedCenter", 5.0f, height);
    _speedCenter->setColor(0.9f, 0.9f, 0.9f, 1.0f);
	
	_speedLabelBox = new osgWidget::Box("labelBox", osgWidget::Box::VERTICAL);
	
    _speedLabel = new osgWidget::Label("label");

    _speedLabel->setFont("fonts/Vera.ttf");
    _speedLabel->setFontSize(24);
    _speedLabel->setFontColor(FONT_COLOR);
    _speedLabel->setAlignHorizontal(osgWidget::Widget::HA_LEFT);
	
    _speedLabelBox->addWidget(_speedLabel);
    _speedLabelBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	_speedBox->addWidget(_speedLeft);
	_speedFrontBox->addWidget(_speedCenter);
	
	_speedLeft->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_speedDown, this, osgWidget::EVENT_MOUSE_PUSH));
	_speedLeft->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_speedDrag, this, osgWidget::EVENT_MOUSE_DRAG));
	_speedLeft->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_speedUp, this, osgWidget::EVENT_MOUSE_RELEASE));
	
	//_sliderPos = _sliderPercent = 0.0f;
	
	
	_speedBox->getBackground()->setColor(0.0f, 1.0f, 0.0f, 0.5f);
    //_buttonBox->setEventMask(osgWidget::EVENT_MASK_MOUSE_DRAG);
    //_buttonBox->addCallback(new osgWidget::Callback(&eatDrag, osgWidget::EVENT_MOUSE_DRAG));
}

bool AnimtkViewerGUI::_speedDown(osgWidget::Event& ev) 
{
	_speedPos = ev.x;
	_setSpeedPos(ev.x);
}



bool AnimtkViewerGUI::_speedDrag(osgWidget::Event& ev) 
{
	
	_speedPos += ev.x;
	_setSpeedPos(_speedPos);
}

bool AnimtkViewerGUI::_speedUp(osgWidget::Event& ev) 
{
	
}

float possibleSpeed[SPEED_SLOTS] = {0.0f, 0.05f, 0.1f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 10.0f};

void AnimtkViewerGUI::_setSpeedPos(float pos)
{	
	if (pos < 0) pos = 0;
	if (pos >= _speedBox->getWidth()) pos = _speedBox->getWidth()-1;
	
	_speedPercent = pos / (_speedBox->getWidth()-1);
	
	//printf("Percent %.2f, ", _speedPercent);
	
	
	// Convertir le pourcentage en entier de 0 à 12
	int slot = (int)(_speedPercent * (SPEED_SLOTS-1) + 0.5f);
	if (slot < 0) slot = 0;
	if (slot >= SPEED_SLOTS) slot = SPEED_SLOTS - 1;
	
	// Reconvertir le slot en pourcentage...
	_speedPercent = slot/(float)(SPEED_SLOTS-1);
	
	//printf("Pos %.2f => Slot %i, Percent %.2f, Speed %f\n", pos, slot, _speedPercent, possibleSpeed[slot]);
	
	char buffer[256];
	//sprintf(buffer, "x%.2f", possibleSpeed[slot]);
	_speedLabel->setLabel(buffer);
	
	// Ajuster le speedPercent
	
	_speedFrontBox->setX(_speedPercent*(_speedBox->getWidth()-5));
	resizeAllWindows();
	
	AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
	osgAnimation::Animation *a = mc.getCurrentAnimation();
	
	
	osgViewer::ViewerExt* ve = ((osgViewer::ViewerExt*)getView());
	ve->setSpeed(possibleSpeed[slot]);
	
	//printf("%f%%\n", _speedPercent*100);
}


void AnimtkViewerGUI::_setSliderTime(float time)
{
	//printf("Time %f\n", time);
	float pos;
	AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
	osgAnimation::Animation *a = mc.getCurrentAnimation();

	
	pos = (time / a->getDuration()) * getWidth();
	
	
	if (time < 0) time = 0;
	else if (time > a->getDuration()) time = a->getDuration();
	
	
	char buffer[256];
	sprintf(buffer, "%02d:%02d.%03d", (int)(time/60), ((int)time)%60, ((int)(time*1000))%1000);
	_timeLabel->setLabel(buffer);
	
	_setSliderPos(pos);
	
}


void AnimtkViewerGUI::_setSliderPos(float pos)
{
	//printf("Pos : %f\n", pos);
	_sliderPos = pos;
	
	//if (pos < 0) pos = 0;
	//else if (pos >= getWidth()-1) pos = getWidth()-1;
	
	_sliderPercent = pos / (getWidth());
	
	_sliderFrontBox->setX(_sliderPercent*(getWidth()-5));
	resizeAllWindows();
	
	AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
	osgAnimation::Animation *a = mc.getCurrentAnimation();
	
	
	osgViewer::ViewerExt* ve = ((osgViewer::ViewerExt*)getView());
	ve->setCurrentTime(a->getDuration()*_sliderPercent);
	
	//printf("%f%%\n", _sliderPercent*100);
}



int AnimtkViewerGUI::getSpeedSlot(void)
{
	int i;
	osgViewer::ViewerExt* ve = ((osgViewer::ViewerExt*)getView());
	_setSliderTime(ve->getCurrentTime());
	
	for (i=0; i<SPEED_SLOTS-1; i++)
		if (ve->getSpeed() < (possibleSpeed[i] + possibleSpeed[i+1])/2)
			break;
	return i;
}

void AnimtkViewerGUI::setSpeedSlot(int i)
{
	_setSpeedPos((i/(float)(SPEED_SLOTS-1))*(_speedBox->getWidth()-1));
}


void AnimtkViewerGUI::accept(osg::NodeVisitor &nv)
{
	if (_hasAnim){
		setSpeedSlot(getSpeedSlot());		// Forcer la position
		
		// Update frame number...
		osgViewer::ViewerExt* ve = ((osgViewer::ViewerExt*)getView());
		char buffer[256];
		sprintf(buffer, "Frame %05d", ve->getFrame());
		_frameLabel->setLabel(buffer);
		
		_sliderBox->resizePercent(100.0f);
		_buttonBox->resizePercent(100.0f);
	}
	WindowManager::accept(nv);
}



bool AnimtkViewerGUI::_sliderDown(osgWidget::Event& ev) 
{
	// Pause while moving
	osgViewer::ViewerExt* ve = ((osgViewer::ViewerExt*)getView());
	ve->setPause(true);
	
	_setSliderPos(ev.x);
}


bool AnimtkViewerGUI::_sliderDrag(osgWidget::Event& ev) 
{
	_setSliderPos(_sliderPos+ev.x);
}

bool AnimtkViewerGUI::_sliderUp(osgWidget::Event& ev) 
{

}

void AnimtkViewerGUI::_createSlider() {
	float height = 25.0f;
    _sliderBox = new osgWidget::Box("sliderBox", osgWidget::Box::HORIZONTAL);
	_sliderFrontBox = new osgWidget::Box("sliderFrontBox", osgWidget::Box::HORIZONTAL);

	_sliderLeft= new osgWidget::NullWidget("_sliderLeft", 5.0f, height);
    _sliderLeft->setColor(0.0f, 0.0f, 0.9f, 0.5f);
	_sliderLeft->setCanFill(true);
	
	_sliderCenter= new osgWidget::NullWidget("_sliderCenter", 5.0f, height);
    _sliderCenter->setColor(0.9f, 0.9f, 0.9f, 1.0f);
	
	_sliderBox->addWidget(_sliderLeft);
	_sliderFrontBox->addWidget(_sliderCenter);
	//_sliderBox->addWidget(_sliderRight);
	
	
	//_sliderLeft->setWidth(400.0f);
	//_sliderLeft->setCanFill(true);
	//_sliderRight->setCanFill(true);
	
	_sliderLeft->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_sliderDown, this, osgWidget::EVENT_MOUSE_PUSH));
	_sliderLeft->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_sliderDrag, this, osgWidget::EVENT_MOUSE_DRAG));
	_sliderLeft->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_sliderUp, this, osgWidget::EVENT_MOUSE_RELEASE));

	_sliderPos = _sliderPercent = 0.0f;
	

	_sliderBox->getBackground()->setColor(0.0f, 1.0f, 0.0f, 0.5f);
    //_buttonBox->setEventMask(osgWidget::EVENT_MASK_MOUSE_DRAG);
    //_buttonBox->addCallback(new osgWidget::Callback(&eatDrag, osgWidget::EVENT_MOUSE_DRAG));


	_timeLabelBox = new osgWidget::Box("labelBox", osgWidget::Box::VERTICAL);
	
    _timeLabel = new osgWidget::Label("label");
	

    _timeLabel->setFont("fonts/Vera.ttf");
    _timeLabel->setFontSize(24);
    _timeLabel->setFontColor(FONT_COLOR);
    _timeLabel->setAlignHorizontal(osgWidget::Widget::HA_LEFT);
	
    _timeLabelBox->addWidget(_timeLabel);
    _timeLabelBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

	
	
	_frameLabelBox = new osgWidget::Box("labelBox", osgWidget::Box::VERTICAL);
	
    _frameLabel = new osgWidget::Label("label");
	
    _frameLabel->setFont("fonts/Vera.ttf");
    _frameLabel->setFontSize(24);
    _frameLabel->setFontColor(FONT_COLOR);
    _frameLabel->setAlignHorizontal(osgWidget::Widget::HA_LEFT);
	
    _frameLabelBox->addWidget(_frameLabel);
    _frameLabelBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);
}


osgWidget::Widget* AnimtkViewerGUI::_createButton(const std::string& name) 
{
    osgWidget::Widget* b = new osgWidget::Widget(name, 64.0f, 64.0f);
    
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

bool AnimtkViewerGUI::_listMouseHover(osgWidget::Event& ev) 
{
    osgWidget::Label* l = dynamic_cast<osgWidget::Label*>(ev.getWidget());

    if(!l) return false;

    if(ev.type == osgWidget::EVENT_MOUSE_ENTER) l->setFontColor(1.0f, 1.0f, 1.0f, 1.0f);

    else if(ev.type == osgWidget::EVENT_MOUSE_LEAVE) l->setFontColor(1.0f, 1.0f, 1.0f, 0.3f);

    else if(ev.type == osgWidget::EVENT_MOUSE_PUSH) {
        AnimtkViewerModelController::instance().playByName(ev.getWidget()->getName());
    }
    
    else return false;

    return true;
}

bool AnimtkViewerGUI::_buttonPush(osgWidget::Event& ev) 
{
	
	

	
    if(!ev.getWidget()) return false;

    osgWidget::Label* l = static_cast<osgWidget::Label*>(_labelBox->getByName("label"));

    if(!l) return false;

    LabelFunctor* lf = dynamic_cast<LabelFunctor*>(l->getUpdateCallback());

    if(!lf) return false;

    // We're safe at this point, so begin processing.
    AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
    std::string                  name = ev.getWidget()->getName();
	osgViewer::ViewerExt* ve = ((osgViewer::ViewerExt*)getView());
	
    if(name == "play") 
	{
		ve->setPause(false);
	}

    else if(name == "stop") {
		ve->setCurrentTime(0.0f);
		ve->setSpeed(1.0f);
		ve->setPause(true);
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
		ve->setPause(true);
		ve->setFrame(ve->getFrame()+1);
	}
	
	else if(name == "back")
	{
		ve->setPause(true);
		ve->setFrame(ve->getFrame()-1);
	}
    else if(name == "pause") 
	{
		ve->setPause(true);
	}

    else return false;

    return true;
}

void AnimtkViewerGUI::_createButtonBox() 
{
    _buttonBox = new osgWidget::Box("buttonBox", osgWidget::Box::HORIZONTAL);

    osgWidget::Widget* space = new osgWidget::Widget("nullSpace", 0.0f, 0.0f);
    osgWidget::Widget* back  = _createButton("back");
    osgWidget::Widget* next  = _createButton("next");
	osgWidget::Widget* ral  = _createButton("ral");
    osgWidget::Widget* play  = _createButton("play");
    osgWidget::Widget* pause = _createButton("pause");
	osgWidget::Widget* rap  = _createButton("rap");
    osgWidget::Widget* stop  = _createButton("stop");
    //osgWidget::Widget* open  = _createButton("open");


    space->setCanFill(true);
    space->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    _buttonBox->addWidget(space);

	_buttonBox->addWidget(ral);
	_buttonBox->addWidget(rap);
    _buttonBox->addWidget(play);
    _buttonBox->addWidget(pause);
    _buttonBox->addWidget(back);
    _buttonBox->addWidget(next);

    _buttonBox->addWidget(stop);
    //_buttonBox->addWidget(open);
    _buttonBox->addWidget(osg::clone(space, "space1", osg::CopyOp::DEEP_COPY_ALL));
    _buttonBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.7f);
    
    _buttonBox->setEventMask(osgWidget::EVENT_MASK_MOUSE_DRAG);
    _buttonBox->addCallback(new osgWidget::Callback(&eatDrag, osgWidget::EVENT_MOUSE_DRAG));
}

void AnimtkViewerGUI::_createListBox() 
{
    _listBox = new osgWidget::Box("listBox", osgWidget::Box::VERTICAL);

    const AnimtkViewerModelController::AnimationMapVector& amv =
        AnimtkViewerModelController::instance().getAnimationMap()
        ;

    for(
        AnimtkViewerModelController::AnimationMapVector::const_iterator i = amv.begin();
        i != amv.end();
        i++
        ) 
{
        osgWidget::Label* label = new osgWidget::Label(*i);

        label->setCanFill(true);
        label->setFont("fonts/Vera.ttf");
        label->setFontSize(15);
        label->setFontColor(1.0f, 1.0f, 1.0f, 0.3f);
        label->setPadding(5.0f);
        label->setAlignHorizontal(osgWidget::Widget::HA_RIGHT);
        label->setLabel(*i);
        label->setEventMask(osgWidget::EVENT_MASK_MOUSE_DRAG);
        label->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_listMouseHover, this, osgWidget::EVENT_MOUSE_ENTER));
        label->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_listMouseHover, this, osgWidget::EVENT_MOUSE_LEAVE));
        label->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_listMouseHover, this, osgWidget::EVENT_MOUSE_PUSH));

        _listBox->addWidget(label);
    }

    ListFunctor* lf = new ListFunctor();

    _listBox->setUpdateCallback(lf);
    _listBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.7f);
}

void AnimtkViewerGUI::_createLabelBox() 
{
    _labelBox = new osgWidget::Box("labelBox", osgWidget::Box::VERTICAL);

    osgWidget::Label* label = new osgWidget::Label("label");
    
    label->setFont("fonts/Vera.ttf");
    label->setFontSize(50);
    label->setFontColor(0.0f, 0.0f, 0.0f, 0.7f);
    label->setAlignHorizontal(osgWidget::Widget::HA_LEFT);
    label->setPadding(10.0f);

    LabelFunctor* lf = new LabelFunctor();
    label->setUpdateCallback(lf);

    _labelBox->addWidget(label);
    _labelBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);
}
