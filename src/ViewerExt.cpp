/*
 *  Viewer_ext.cpp
 *  OpenSceneGraph
 *
 *  Created by Benjamin Bozou on 18/02/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ViewerExt"

#include <stdio.h>

#include <osgAnimation/Channel>
#include <osgAnimation/Sampler>


using namespace osgViewer;

ViewerExt::ViewerExt():
_speed (1.0f),
_currentTime(0.0f),
_totalTime(0.0f),
_currentFrame(0),
_totalFrame(0),
_pause(true)
{

}

void ViewerExt::setTimelineFromAnimation(osgAnimation::Animation* animation)
{
    _totalTime = animation->getDuration();
    
    osgAnimation::ChannelList channels = animation->getChannels();
    if (channels.size() > 0)
    {
        osgAnimation::FloatCubicBezierKeyframeContainer *kfc = (osgAnimation::FloatCubicBezierKeyframeContainer*)(channels[0]->getSampler()->getKeyframeContainer());
        _totalFrame = kfc->size();
        frameTimes = new float[_totalFrame];
        for (int i=0; i<_totalFrame; i++)
        {
            frameTimes[i] = (*kfc)[i].getTime() - animation->getStartTime();
        }
        std::cout<<"Use first Channel to define frameTimes.\nThis animation has a duration of : "<<_totalTime<<" s with "<<_totalFrame<<" frames"<<std::endl;
    }
    else
    {
        _totalFrame = 1;
        frameTimes = new float[_totalFrame];
        frameTimes[0] = 0.;
        std::cout<<"Problem while loading animation: No channel found"<<std::endl;
    }
}


int ViewerExt::run()
{
    if (!isRealized())
    {
        realize();
    }

    osg::Timer_t startFrameTick = osg::Timer::instance()->tick();
    osg::Timer_t endFrameTick;
    double frameTime;
    while(!done())
    {
        frame(_currentTime);

        endFrameTick = osg::Timer::instance()->tick();
        frameTime = osg::Timer::instance()->delta_s(startFrameTick, endFrameTick);
        startFrameTick = osg::Timer::instance()->tick();

        if (!_pause)
        {
            _currentTime += frameTime * _speed;
        }
        if (!_pause && _speed && (_currentTime > _totalTime))
        {
            _currentTime = 0.0f;
        }
        
    }

    return 0;
}




void ViewerExt::setPause(bool state)
{
    _pause = state;
}

bool ViewerExt::getPause()
{
    return _pause;
}



void ViewerExt::setSpeed(float speed)
{
    _speed = speed;
}

float ViewerExt::getSpeed()
{
    return _speed;
}



void ViewerExt::setCurrentTime(float time)
{
    _currentTime = time;
}

float ViewerExt::getCurrentTime()
{
    return _currentTime;
}



int ViewerExt::getFrame(void)
{
    for (int i=0; i<_totalFrame - 1; i++)
    {
        if ((frameTimes[i] <= _currentTime) && (_currentTime < frameTimes[i+1]))
        {
            return i;
        }
    }
    return _totalFrame - 1; // It is the last frame
}

void ViewerExt::setFrame(int frame)
{
    _currentFrame = frame;
    if (_currentFrame >= _totalFrame)
    {
        _currentFrame = 0;       // Boucle
    }
    else if (_currentFrame < 0)
    {
        _currentFrame = _totalFrame - 1;    // Boucle
    }
    _currentTime = frameTimes[_currentFrame];
}




