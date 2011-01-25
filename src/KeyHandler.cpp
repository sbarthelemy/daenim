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
    _actionKeys["toggle_inertia"]  = 'i';
    _actionKeys["toggle_names"]  = 'n';
    //_actionKeys["toggle_interface"]  = 'm';
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
            return true;
        }
        else if (ea.getKey() == _actionKeys["toggle_shapes"])
        {
            mask ^= (1 << 1);
            camera->setCullMask(mask);
            return true;
        }
        else if (ea.getKey() == _actionKeys["toggle_links"])
        {
            mask ^= (1 << 2);
            camera->setCullMask(mask);
            return true;
        }
        else if (ea.getKey() == _actionKeys["toggle_inertia"])
        {
            mask ^= (1 << 3);
            camera->setCullMask(mask);
            return true;
        }
        else if (ea.getKey() == _actionKeys["toggle_names"])
        {
            mask ^= (1 << 4);
            camera->setCullMask(mask);
            return true;
        }
        //else if (ea.getKey() == _actionKeys[Play]) return mc.play();
        return false;
    }

    return false;
}
