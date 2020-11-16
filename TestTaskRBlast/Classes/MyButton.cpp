/****************************************************************************
Copyright (c) 2020 Anton Kulikov
****************************************************************************/

#include "2d/CCCamera.h"
#include "MyButton.h"


static const int NORMAL_RENDERER_Z = (-2);
static const int PRESSED_RENDERER_Z = (-2);
static const int DISABLED_RENDERER_Z = (-2);
static const int TITLE_RENDERER_Z = (-1);
static const float ZOOM_ACTION_TIME_STEP = 0.05f;


IMPLEMENT_CLASS_GUI_INFO(MyButton)

MyButton::MyButton():
_buttonState(ButtonState::IDLE),
_buttonIdleRenderer(nullptr),
_buttonPushedRenderer(nullptr),
_buttonDragoutRenderer(nullptr),
_zoomScale(0.1f),
_prevIgnoreSize(true),
_scale9Enabled(false),
_capInsetsIdle(cocos2d::Rect::ZERO),
_capInsetsPushed(cocos2d::Rect::ZERO),
_capInsetsDragout(cocos2d::Rect::ZERO),
_idleTextureSize(_contentSize),
_pushedTextureSize(_contentSize),
_dragoutTextureSize(_contentSize),
_idleTextureLoaded(false),
_pushedTextureLoaded(false),
_dragoutTextureLoaded(false),
_idleTextureAdaptDirty(true),
_pushedTextureAdaptDirty(true),
_dragoutTextureAdaptDirty(true),
_idleFileName(""),
_pushedFileName(""),
_dragoutFileName(""),
_idleTexType(TextureResType::LOCAL),
_pushedTexType(TextureResType::LOCAL),
_dragoutTexType(TextureResType::LOCAL),
_expandZone(cocos2d::Size::ZERO),
_safeZone(cocos2d::Size::ZERO),
_pushedTimeout(0.0f),
_pushedTime(0.0f)
{
    setTouchEnabled(true);
	this->scheduleUpdate();
}

MyButton::~MyButton()
{
}

MyButton* MyButton::create()
{
    MyButton* widget = new (std::nothrow) MyButton();
    if (widget && widget->init())
    {
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}

MyButton* MyButton::create(const std::string &idleImage,
                       const std::string& selectedImage ,
                       const std::string& disableImage,
                       TextureResType texType)
{
    MyButton *btn = new (std::nothrow) MyButton;
    if (btn && btn->init(idleImage,selectedImage,disableImage,texType))
    {
        btn->autorelease();
        return btn;
    }
    CC_SAFE_DELETE(btn);
    return nullptr;
}

bool MyButton::init(const std::string &idleImage,
                  const std::string& selectedImage ,
                  const std::string& disableImage,
                  TextureResType texType)
{

    // invoke an overridden init() at first
    if (!Widget::init()) {
        return false;
    }

    loadTextures(idleImage, selectedImage, disableImage, texType);
	_refreshButtonState(nullptr);

    return true;
}

bool MyButton::init()
{
    if (Widget::init())
    {
        return true;
    }
    return false;
}

void MyButton::initRenderer()
{
    _buttonIdleRenderer = cocos2d::ui::Scale9Sprite::create();
    _buttonPushedRenderer = cocos2d::ui::Scale9Sprite::create();
    _buttonDragoutRenderer = cocos2d::ui::Scale9Sprite::create();
    _buttonPushedRenderer->setRenderingType(cocos2d::ui::Scale9Sprite::RenderingType::SIMPLE);
    _buttonIdleRenderer->setRenderingType(cocos2d::ui::Scale9Sprite::RenderingType::SIMPLE);
    _buttonDragoutRenderer->setRenderingType(cocos2d::ui::Scale9Sprite::RenderingType::SIMPLE);

    addProtectedChild(_buttonIdleRenderer, NORMAL_RENDERER_Z, -1);
    addProtectedChild(_buttonPushedRenderer, PRESSED_RENDERER_Z, -1);
    addProtectedChild(_buttonDragoutRenderer, DISABLED_RENDERER_Z, -1);
}
    
void MyButton::addButtonChild(cocos2d::Node* child)
{
	ChildInfo info;
	info.node = child;
	info.state_any = true;
	_childNodes.push_back(info);
	addChild(child);
	_updateChildren();
}

void MyButton::addButtonChild(cocos2d::Node* child, ButtonState state)
{
	ChildInfo info;
	info.node = child;
	info.state = state;
	info.state_any = false;
	_childNodes.push_back(info);
	addChild(child);
	_updateChildren();
}

void MyButton::removeButtonChild(cocos2d::Node* child)
{
	for (auto i = _childNodes.begin(); i != _childNodes.end(); ++i)
	{
		if (i->node != child)
			continue;
		_childNodes.erase(i);
		child->removeFromParent();
		break;
	}
}


void MyButton::setScale9Enabled(bool able)
{
    if (_scale9Enabled == able)
    {
        return;
    }

    _scale9Enabled = able;

    if (_scale9Enabled) {
        _buttonIdleRenderer->setRenderingType(cocos2d::ui::Scale9Sprite::RenderingType::SLICE);
        _buttonPushedRenderer->setRenderingType(cocos2d::ui::Scale9Sprite::RenderingType::SLICE);
        _buttonDragoutRenderer->setRenderingType(cocos2d::ui::Scale9Sprite::RenderingType::SLICE);
    }else{
        _buttonIdleRenderer->setRenderingType(cocos2d::ui::Scale9Sprite::RenderingType::SIMPLE);
        _buttonPushedRenderer->setRenderingType(cocos2d::ui::Scale9Sprite::RenderingType::SIMPLE);
        _buttonDragoutRenderer->setRenderingType(cocos2d::ui::Scale9Sprite::RenderingType::SIMPLE);
    }
    

    if (_scale9Enabled)
    {
        bool ignoreBefore = _ignoreSize;
        ignoreContentAdaptWithSize(false);
        _prevIgnoreSize = ignoreBefore;
    }
    else
    {
        ignoreContentAdaptWithSize(_prevIgnoreSize);
    }

    setCapInsetsIdleRenderer(_capInsetsIdle);
    setCapInsetsPushedRenderer(_capInsetsPushed);
    setCapInsetsDragoutRenderer(_capInsetsDragout);

    _brightStyle = BrightStyle::NONE;
    setBright(_bright);

    _idleTextureAdaptDirty = true;
    _pushedTextureAdaptDirty = true;
    _dragoutTextureAdaptDirty = true;
}

bool MyButton::isScale9Enabled()const
{
    return _scale9Enabled;
}

void MyButton::ignoreContentAdaptWithSize(bool ignore)
{
    if (_unifySize)
    {
        this->updateContentSize();
        return;
    }

    if (!_scale9Enabled || (_scale9Enabled && !ignore))
    {
        Widget::ignoreContentAdaptWithSize(ignore);
        _prevIgnoreSize = ignore;
    }
}

void MyButton::loadTextures(const std::string& idle,
                          const std::string& selected,
                          const std::string& dragout,
                          TextureResType texType)
{
    loadTextureIdle(idle,texType);
    loadTexturePushed(selected,texType);
    loadTextureDragout(dragout,texType);
}

void MyButton::loadTextureIdle(const std::string& idle,TextureResType texType)
{
    _idleFileName = idle;
    _idleTexType = texType;
    bool textureLoaded = true;
    if (idle.empty())
    {
        _buttonIdleRenderer->resetRender();
        textureLoaded = false;
    }
    else
    {
        switch (texType)
        {
        case TextureResType::LOCAL:
            _buttonIdleRenderer->initWithFile(idle);
            break;
        case TextureResType::PLIST:
            _buttonIdleRenderer->initWithSpriteFrameName(idle);
            break;
        default:
            break;
        }
    }
    //FIXME: https://github.com/cocos2d/cocos2d-x/issues/12249
    if (!_ignoreSize && _customSize.equals(cocos2d::Size::ZERO)) {
        _customSize = _buttonIdleRenderer->getContentSize();
    }
    this->setupIdleTexture(textureLoaded);
}

void MyButton::setupIdleTexture(bool textureLoaded)
{
    _idleTextureSize = _buttonIdleRenderer->getContentSize();

    this->updateChildrenDisplayedRGBA();

    if (_unifySize )
    {
        if (!_scale9Enabled)
        {
            updateContentSizeWithTextureSize(this->getIdleSize());
        }
    }
    else
    {
        updateContentSizeWithTextureSize(_idleTextureSize);
    }
    _idleTextureLoaded = textureLoaded;
    _idleTextureAdaptDirty = true;
}

void MyButton::loadTextureIdle(cocos2d::SpriteFrame* idleSpriteFrame)
{
    _buttonIdleRenderer->initWithSpriteFrame(idleSpriteFrame);
    this->setupIdleTexture(nullptr != idleSpriteFrame);
}

void MyButton::loadTexturePushed(const std::string& selected,TextureResType texType)
{
    _pushedFileName = selected;
    _pushedTexType = texType;
    bool textureLoaded = true;
    if (selected.empty())
    {
        _buttonPushedRenderer->resetRender();
        textureLoaded = false;
    }
    else
    {
        switch (texType)
        {
        case TextureResType::LOCAL:
            _buttonPushedRenderer->initWithFile(selected);
            break;
        case TextureResType::PLIST:
            _buttonPushedRenderer->initWithSpriteFrameName(selected);
            break;
        default:
            break;
        }
    }
    this->setupPushedTexture(textureLoaded);
}

void MyButton::setupPushedTexture(bool textureLoaded)
{
    _pushedTextureSize = _buttonPushedRenderer->getContentSize();

    this->updateChildrenDisplayedRGBA();

    _pushedTextureLoaded = textureLoaded;
    _pushedTextureAdaptDirty = true;
}

void MyButton::loadTexturePushed(cocos2d::SpriteFrame* pushedSpriteFrame)
{
    _buttonPushedRenderer->initWithSpriteFrame(pushedSpriteFrame);
    this->setupPushedTexture(nullptr != pushedSpriteFrame);
}

void MyButton::loadTextureDragout(const std::string& dragout,TextureResType texType)
{
    _dragoutFileName = dragout;
    _dragoutTexType = texType;
    bool textureLoaded = true;
    if (dragout.empty())
    {
        _buttonDragoutRenderer->resetRender();
        textureLoaded = false;
    }
    else
    {
        switch (texType)
        {
        case TextureResType::LOCAL:
            _buttonDragoutRenderer->initWithFile(dragout);
            break;
        case TextureResType::PLIST:
            _buttonDragoutRenderer->initWithSpriteFrameName(dragout);
            break;
        default:
            break;
        }
    }
    this->setupDragoutTexture(textureLoaded);
}

void MyButton::setupDragoutTexture(bool textureLoaded)
{
    _dragoutTextureSize = _buttonDragoutRenderer->getContentSize();

    this->updateChildrenDisplayedRGBA();

    _dragoutTextureLoaded = textureLoaded;
    _dragoutTextureAdaptDirty = true;
}

void MyButton::loadTextureDragout(cocos2d::SpriteFrame* dragoutSpriteFrame)
{
    _buttonDragoutRenderer->initWithSpriteFrame(dragoutSpriteFrame);
    this->setupDragoutTexture(nullptr != dragoutSpriteFrame);
}

void MyButton::setCapInsets(const cocos2d::Rect &capInsets)
{
    setCapInsetsIdleRenderer(capInsets);
    setCapInsetsPushedRenderer(capInsets);
    setCapInsetsDragoutRenderer(capInsets);
}


void MyButton::setCapInsetsIdleRenderer(const cocos2d::Rect &capInsets)
{
    _capInsetsIdle = cocos2d::ui::Helper::restrictCapInsetRect(capInsets, this->_idleTextureSize);

    //for performance issue
    if (!_scale9Enabled)
    {
        return;
    }
    _buttonIdleRenderer->setCapInsets(_capInsetsIdle);
}

void MyButton::setCapInsetsPushedRenderer(const cocos2d::Rect &capInsets)
{
    _capInsetsPushed = cocos2d::ui::Helper::restrictCapInsetRect(capInsets, this->_pushedTextureSize);

    //for performance issue
    if (!_scale9Enabled)
    {
        return;
    }
    _buttonPushedRenderer->setCapInsets(_capInsetsPushed);
}

void MyButton::setCapInsetsDragoutRenderer(const cocos2d::Rect &capInsets)
{
    _capInsetsDragout = cocos2d::ui::Helper::restrictCapInsetRect(capInsets, this->_dragoutTextureSize);

    //for performance issue
    if (!_scale9Enabled)
    {
        return;
    }
    _buttonDragoutRenderer->setCapInsets(_capInsetsDragout);
}

const cocos2d::Rect& MyButton::getCapInsetsIdleRenderer()const
{
    return _capInsetsIdle;
}

const cocos2d::Rect& MyButton::getCapInsetsPushedRenderer()const
{
    return _capInsetsPushed;
}

const cocos2d::Rect& MyButton::getCapInsetsDragoutRenderer()const
{
    return _capInsetsDragout;
}

void MyButton::updateContentSize()
{
    if (_unifySize)
    {
        if (_scale9Enabled)
        {
            ProtectedNode::setContentSize(_customSize);
        }
        else
        {
			cocos2d::Size s = getIdleSize();
            ProtectedNode::setContentSize(s);
        }
        onSizeChanged();
        return;
    }

    if (_ignoreSize)
    {
        this->setContentSize(getVirtualRendererSize());
    }
}

void MyButton::onSizeChanged()
{
    Widget::onSizeChanged();
	for (auto& info : _childNodes)
	{
		// TODO
		/*
		if (nullptr != _titleRenderer)
		{
			_titleRenderer->setPosition(_contentSize.width * 0.5f, _contentSize.height * 0.5f);
			updateTitleLocation();
		}*/
	}
    _idleTextureAdaptDirty = true;
    _pushedTextureAdaptDirty = true;
    _dragoutTextureAdaptDirty = true;
}


void MyButton::update(float dt)
{
	Widget::update(dt);
	if (_buttonState == ButtonState::PUSHED)
	{
		_pushedTime += dt;
		if (_pushedTimeout != 0.0 && _pushedTime >= _pushedTimeout)
		{
			_refreshButtonState(nullptr);
			_generateEvent(ButtonState::PUSHED, true);
		}
	}
}


void MyButton::adaptRenderers()
{
    if (_idleTextureAdaptDirty)
    {
        idleTextureScaleChangedWithSize();
        _idleTextureAdaptDirty = false;
    }

    if (_pushedTextureAdaptDirty)
    {
        pushedTextureScaleChangedWithSize();
        _pushedTextureAdaptDirty = false;
    }

    if (_dragoutTextureAdaptDirty)
    {
        dragoutTextureScaleChangedWithSize();
        _dragoutTextureAdaptDirty = false;
    }
}

cocos2d::Size MyButton::getVirtualRendererSize() const
{
    if (_unifySize)
    {
        return this->getIdleSize();
    }
	// TODO
	/*
    if (nullptr != _titleRenderer)
    {
		cocos2d::Size titleSize = _titleRenderer->getContentSize();
        if (!_idleTextureLoaded && _titleRenderer->getString().size() > 0)
        {
            return titleSize;
        }
    }*/
    return _idleTextureSize;
}

cocos2d::Node* MyButton::getVirtualRenderer()
{
    if (_bright)
    {
        switch (_brightStyle)
        {
            case BrightStyle::NORMAL:
                return _buttonIdleRenderer;
            case BrightStyle::HIGHLIGHT:
                return _buttonPushedRenderer;
            default:
                return nullptr;
        }
    }
    else
    {
        return _buttonDragoutRenderer;
    }
}

void MyButton::idleTextureScaleChangedWithSize()
{
    _buttonIdleRenderer->setPreferredSize(_contentSize);

    _buttonIdleRenderer->setPosition(_contentSize.width / 2.0f, _contentSize.height / 2.0f);
}

void MyButton::pushedTextureScaleChangedWithSize()
{
    _buttonPushedRenderer->setPreferredSize(_contentSize);

    _buttonPushedRenderer->setPosition(_contentSize.width / 2.0f, _contentSize.height / 2.0f);
}

void MyButton::dragoutTextureScaleChangedWithSize()
{
    _buttonDragoutRenderer->setPreferredSize(_contentSize);
    
    _buttonDragoutRenderer->setPosition(_contentSize.width / 2.0f, _contentSize.height / 2.0f);
}

void MyButton::setZoomScale(float scale)
{
    _zoomScale = scale;
}

float MyButton::getZoomScale()const
{
    return _zoomScale;
}

std::string MyButton::getDescription() const
{
    return "MyButton";
}

cocos2d::ui::Widget* MyButton::createCloneInstance()
{
    return MyButton::create();
}

void MyButton::copySpecialProperties(Widget *widget)
{
    MyButton* button = dynamic_cast<MyButton*>(widget);
    if (button)
    {
        _prevIgnoreSize = button->_prevIgnoreSize;
        setScale9Enabled(button->_scale9Enabled);

        // clone the inner sprite: https://github.com/cocos2d/cocos2d-x/issues/16924
        button->_buttonIdleRenderer->copyTo(_buttonIdleRenderer);
        _idleFileName = button->_idleFileName;
        _idleTextureSize = button->_idleTextureSize;
        _idleTexType = button->_idleTexType;
        _idleTextureLoaded = button->_idleTextureLoaded;
        setupIdleTexture(!_idleFileName.empty());

        button->_buttonPushedRenderer->copyTo(_buttonPushedRenderer);
        _pushedFileName = button->_pushedFileName;
        _pushedTextureSize = button->_pushedTextureSize;
        _pushedTexType = button->_pushedTexType;
        _pushedTextureLoaded = button->_pushedTextureLoaded;
        setupPushedTexture(!_pushedFileName.empty());

        button->_buttonDragoutRenderer->copyTo(_buttonDragoutRenderer);
        _dragoutFileName = button->_dragoutFileName;
        _dragoutTextureSize = button->_dragoutTextureSize;
        _dragoutTexType = button->_dragoutTexType;
        _dragoutTextureLoaded = button->_dragoutTextureLoaded;
        setupDragoutTexture(!_dragoutFileName.empty());

        setCapInsetsIdleRenderer(button->_capInsetsIdle);
        setCapInsetsPushedRenderer(button->_capInsetsPushed);
        setCapInsetsDragoutRenderer(button->_capInsetsDragout);
        setZoomScale(button->_zoomScale);
    }

}
cocos2d::Size MyButton::getIdleSize() const
{
	cocos2d::Size imageSize;
    if (_buttonIdleRenderer != nullptr)
    {
        imageSize = _buttonIdleRenderer->getContentSize();
    }
	// TODO
	cocos2d::Size titleSize = imageSize;
	/*
	if (_titleRenderer != nullptr)
	{
		titleSize = _titleRenderer->getContentSize();
	}*/
	float width = titleSize.width > imageSize.width ? titleSize.width : imageSize.width;
    float height = titleSize.height > imageSize.height ? titleSize.height : imageSize.height;

    return cocos2d::Size(width,height);
}

bool MyButton::hitTest(const cocos2d::Vec2 &pt,
					   const cocos2d::Camera* camera,
					   cocos2d::Vec3 *p) const
{
	auto rect = getSafeZoneRect();
	return isScreenPointInRect(pt, camera, getWorldToNodeTransform(), rect, p);
}


bool MyButton::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unusedEvent)
{
	auto r = Widget::onTouchBegan(touch, unusedEvent);
	_refreshButtonState(&touch->getLocation());
	return r;
}

void MyButton::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unusedEvent)
{
	Widget::onTouchMoved(touch, unusedEvent);
	_refreshButtonState(&touch->getLocation());
}

void MyButton::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unusedEvent)
{
	auto old_state = _buttonState;
	Widget::onTouchEnded(touch, unusedEvent);
	_refreshButtonState(nullptr);
	if (old_state == ButtonState::PUSHED)
		_generateEvent(old_state, false);
}


void MyButton::_generateEvent(ButtonState state, bool is_long)
{
	if (!_touchEndedCallback)
		return;
	cocos2d::EventCustom event("my_button_touch_ended");
	TouchEndedCallbackData data;
	data.button = this;
	data.state = state;
	event.setUserData(&data);
	_touchEndedCallback(&event);
}

void MyButton::onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unusedEvent)
{
	Widget::onTouchCancelled(touch, unusedEvent);
	_refreshButtonState(nullptr);
}

cocos2d::Size MyButton::getIdleTextureSize() const
{
    return _idleTextureSize;
}

void MyButton::resetIdleRender()
{
    _idleFileName = "";
    _idleTexType = TextureResType::LOCAL;

    _idleTextureSize = cocos2d::Size(0, 0);

    _idleTextureLoaded = false;
    _idleTextureAdaptDirty = false;

    _buttonIdleRenderer->resetRender();
}
void MyButton::resetPushedRender()
{
    _pushedFileName = "";
    _pushedTexType = TextureResType::LOCAL;

    _pushedTextureSize = cocos2d::Size(0, 0);

    _pushedTextureLoaded = false;
    _pushedTextureAdaptDirty = false;

    _buttonPushedRenderer->resetRender();
}

void MyButton::resetDragoutRender()
{
    _dragoutFileName = "";
    _dragoutTexType = TextureResType::LOCAL;

    _dragoutTextureSize = cocos2d::Size(0, 0);

    _dragoutTextureLoaded = false;
    _dragoutTextureAdaptDirty = false;

    _buttonDragoutRenderer->resetRender();
}

cocos2d::ResourceData MyButton::getIdleFile()
{
	cocos2d::ResourceData rData;
    rData.type = (int)_idleTexType;
    rData.file = _idleFileName;
    return rData;
}
cocos2d::ResourceData MyButton::getPushedFile()
{
	cocos2d::ResourceData rData;
    rData.type = (int)_pushedTexType;
    rData.file = _pushedFileName;
    return rData;
}
cocos2d::ResourceData MyButton::getDragoutFile()
{
	cocos2d::ResourceData rData;
    rData.type = (int)_dragoutTexType;
    rData.file = _dragoutFileName;
    return rData;
}

cocos2d::Size MyButton::getExpandZone() const
{
	auto contentSize = getContentSize();
	cocos2d::Size expandZone;
	expandZone.width = std::max(contentSize.width, _expandZone.width);
	expandZone.height = std::max(contentSize.height, _expandZone.height);
	return expandZone;
}
void MyButton::setExpandZone(const cocos2d::Size& expandZone)
{
	_expandZone = expandZone;
}
cocos2d::Rect MyButton::getExpandZoneRect() const
{
	return _makeZoneRect(getExpandZone());
}

cocos2d::Size MyButton::getSafeZone() const
{
	auto expandZone = getExpandZone();
	cocos2d::Size safeZone;
	safeZone.width = std::max(expandZone.width, _safeZone.width);
	safeZone.height = std::max(expandZone.height, _safeZone.height);
	return safeZone;
}
void MyButton::setSafeZone(const cocos2d::Size& safeZone)
{
	_safeZone = safeZone;
}
cocos2d::Rect MyButton::getSafeZoneRect() const
{
	return _makeZoneRect(getSafeZone());
}

cocos2d::Rect MyButton::_makeZoneRect(const cocos2d::Size& zoneSize) const
{
	auto contentSize = getContentSize();
	auto diff = zoneSize - contentSize;
	cocos2d::Rect zoneRect;
	zoneRect.origin = diff / -2.0f;
	zoneRect.size = zoneSize;
	return zoneRect;
}

void MyButton::_refreshButtonState(const cocos2d::Vec2* pt)
{
	if (!pt)
	{
		_buttonState = ButtonState::IDLE;
	}
	else
	{
		auto camera = cocos2d::Camera::getVisitingCamera();
		auto mat = getWorldToNodeTransform();
		auto expandZoneRect = getExpandZoneRect();
		if (isScreenPointInRect(*pt, camera, mat, expandZoneRect, nullptr))
		{
			if (_buttonState != ButtonState::PUSHED)
			{
				_buttonState = ButtonState::PUSHED;
				_pushedTime = 0.0f;
			}
		}
		else
		{
			auto safeZoneRect = getSafeZoneRect();
			if (isScreenPointInRect(*pt, camera, mat, safeZoneRect, nullptr))
				_buttonState = ButtonState::DRAGOUT;
			else
				_buttonState = ButtonState::IDLE;
		}
	}

	_buttonIdleRenderer->setVisible(_buttonState == ButtonState::IDLE);
	_buttonPushedRenderer->setVisible(_buttonState == ButtonState::PUSHED);
	_buttonDragoutRenderer->setVisible(_buttonState == ButtonState::DRAGOUT);
	//_buttonIdleRenderer->setState(cocos2d::ui::Scale9Sprite::State::NORMAL);
	_updateChildren();
}

void MyButton::setTouchEndedCallback(const TouchEndedCallback& cb)
{
	_touchEndedCallback = cb;
}

float MyButton::getPushedTimeout()
{
	return _pushedTimeout;
}
void MyButton::setPushedTimeout(float v)
{
	_pushedTimeout = v;
}

void MyButton::_updateChildren()
{
	for (auto& info : _childNodes)
	{
		if (info.state_any)
			continue;
		info.node->setVisible(_buttonState == info.state);
	}
}
