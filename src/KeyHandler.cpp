/*  -*-c++-*- 
 */

#include <iostream>

#include "KeyHandler"

#include <osgViewer/Viewer>

KeyEventHandler::KeyEventHandler()
{
    _actionKeys["toggle_frames"] = 'f';
    _actionKeys["toggle_shapes"] = 's';
    _actionKeys["toggle_links"]  = 'l';
}


bool KeyEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*)
{

    
    if(ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN) 
    {
        osg::Camera* camera= aa.asView()->getCamera();
        osg::Node::NodeMask mask = camera->getCullMask();
        
        if (ea.getKey() == _actionKeys["toggle_frames"])
        {
            mask ^= (1 << 0);
            camera->setCullMask(mask);
        }
        if (ea.getKey() == _actionKeys["toggle_shapes"])
        {
            mask ^= (1 << 1);
            camera->setCullMask(mask);
        }
        if (ea.getKey() == _actionKeys["toggle_links"])
        {
            mask ^= (1 << 2);
            camera->setCullMask(mask);
        }
        //else if (ea.getKey() == _actionKeys[Play]) return mc.play();
        return true;
    }

    return false;
}
