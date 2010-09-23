/*
 *  Viewer_ext.cpp
 *  OpenSceneGraph
 *
 *  Created by Benjamin Bozou on 18/02/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ViewerExt"
#include "AnimtkViewer"

#include <stdio.h>
#include <stdlib.h>

#include <osg/DeleteHandler>
#include <osg/io_utils>

#include <osgUtil/GLObjectsVisitor>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>

#include <osgViewer/ViewerBase>
#include <osgViewer/View>
#include <osgViewer/Viewer>
#include <osgViewer/Renderer>
#include <osgViewer/CompositeViewer>

#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osg/TexMat>
#include <osg/DeleteHandler>

#include <osgUtil/Optimizer>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/Statistics>

#include <sstream>
#include <string.h>


using namespace osgViewer;
using namespace osgAnimation;

ViewerExt::ViewerExt(osg::ArgumentParser& arguments):
_speed (1.0f),
osgViewer::Viewer(arguments)
{
	//osgViewer::Viewer(arguments);
}

void ViewerExt::setPause(bool state)
{
	_pause = state;
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

float ViewerExt::getCurrentTime(void){
	return _currentTime;
}



int ViewerExt::getFrame(void){
	//printf("Get frame for time %f\n", _currentTime);
	for (int i=0; i<_totalFrames - 1; i++)
	{
		if (_currentTime < frameTimes[i+1] - 0.001) return i;	// Autoriser un delta de marge
	}
	return _totalFrames - 1;		// Arrive a la fin
}


void ViewerExt::setFrame(int frame){
	_currentFrame = frame;
	if (_currentFrame >= _totalFrames) _currentFrame = 0;	// Boucle
	if (_currentFrame < 0) _currentFrame = _totalFrames - 1; // Boucle
	
	//_currentTime = (frame * _totalTime / (_totalFrames-1));  // Valeur approchée, manque de précision
	_currentTime = frameTimes[_currentFrame];
}


int ViewerExt::run(bool hasAnim)
{
    if (!getCameraManipulator() && getCamera()->getAllowEventFocus())
    {
        setCameraManipulator(new osgGA::TrackballManipulator());
	}
	    setReleaseContextAtEndOfFrameHint(false);

	_currentTime = 0.0f;
	_currentFrame = 0;
	_speed = 1.0f;
	_pause = true;
	
	
	if (!isRealized())
	{
		realize();
	}
	
	const char* str = getenv("OSG_RUN_FRAME_COUNT");
	int runTillFrameNumber = str==0 ? -1 : atoi(str);

	if (hasAnim){
		AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
		mc.play();
		ChannelList cl = mc.getCurrentAnimation()->getChannels();
		
		/*
		for(unsigned int i = 0; i < cl.size(); i++) {
			Channel *c = cl[i];
			std::cout << "Channel " << i << " : " << c->getName() << std::endl;
		}
		*/
		
		
		// On prend le premier
		Channel *c = cl[0];
		
		Sampler *s = c->getSampler();
		FloatCubicBezierKeyframeContainer *_currentTKC = (FloatCubicBezierKeyframeContainer*)(s->getKeyframeContainer());
		

		_totalFrames = _currentTKC->size();
		_totalTime = mc.getCurrentAnimation()->getDuration();
		frameTimes = new float[_currentTKC->size()];
		
		std::cout << "Found " << _totalFrames << " frames !" << std::endl;
		//std::cout << "Start time : " << c->getStartTime() << std::endl;
		//std::cout << "End time   : " << c->getEndTime() << std::endl;
		std::cout << "Animation duration : " << _totalTime << std::endl;
				// Afficher tous les temps...
		for (int i=0; i<_totalFrames; i++)
		{
			frameTimes[i] = (*_currentTKC)[i].getTime() - c->getStartTime();
			std::cout << "Frame " << i << " : " << frameTimes[i] << std::endl;
		}
	}

	
	osg::Timer_t startFrameTick = osg::Timer::instance()->tick();
	while(!done() || (runTillFrameNumber>=0 && getViewerFrameStamp()->getFrameNumber()>runTillFrameNumber))
	{
		double minFrameTime = _runMaxFrameRate>0.0 ? 1.0/_runMaxFrameRate : 0.0;
		
		if (_runFrameScheme==ON_DEMAND)
		{
			if (checkNeedToDoFrame()) frame(_currentTime);
		}
		else
		{
			frame(_currentTime);
		}
		
		// work out if we need to force a sleep to hold back the frame rate
		osg::Timer_t endFrameTick = osg::Timer::instance()->tick();
		double frameTime = osg::Timer::instance()->delta_s(startFrameTick, endFrameTick);
		_currentTime += frameTime * _speed * !(_pause);		
		startFrameTick = osg::Timer::instance()->tick();
		if (frameTime < minFrameTime) OpenThreads::Thread::microSleep(static_cast<unsigned int>(1000000.0*(minFrameTime-frameTime)));
	
		if (hasAnim && _speed && !_pause){
			AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
			osgAnimation::Animation *a = mc.getCurrentAnimation();
			
			if (_currentTime >= a->getDuration()){
				_currentTime = 0.0f;
			}
		}
	}
	
	return 0;
}
	