/****************************************************************************
Copyright (c) 2020 Anton Kulikov
****************************************************************************/

#ifndef __MYBUTTON_H__
#define __MYBUTTON_H__

#include <list>
#include "base/CCEventCustom.h"
#include "ui/UIWidget.h"
#include "ui/GUIExport.h"
#include "ui/UIScale9Sprite.h"
#include "2d/CCLabel.h"
#include "2d/CCSprite.h"
#include "2d/CCActionInterval.h"
#include "platform/CCFileUtils.h"
#include "ui/UIHelper.h"
#include <algorithm>
#include "editor-support/cocostudio/CocosStudioExtension.h"


class MyButton : public cocos2d::ui::Widget
{

    DECLARE_CLASS_GUI_INFO

public:
	enum class ButtonState
	{
		IDLE,
		PUSHED,
		DRAGOUT
	};

	/**
     * Default constructor.
     */
    MyButton();

    /**
     * Default destructor.
     *
     * @lua NA
     */
    virtual ~MyButton();

    /**
     * Create a empty MyButton.
     *@return A empty MyButton instance.
     */
    static MyButton* create();

    /**
     * Create a button with custom textures.
     * @param idleImage idle state texture name.
     * @param selectedImage  selected state texture name.
     * @param disableImage dragout state texture name.
     * @param texType    @see `TextureResType`
     * @return a MyButton instance.
     */
    static MyButton* create(const std::string& idleImage,
                          const std::string& selectedImage = "",
                          const std::string& disableImage = "",
                          TextureResType texType = TextureResType::LOCAL);

    /**
     * Load textures for button.
     *
     * @param idle    idle state texture name.
     * @param selected    selected state texture name.
     * @param dragout    dragout state texture name.
     * @param texType    @see `TextureResType`
     */
    void loadTextures(const std::string& idle,
                      const std::string& selected,
                      const std::string& dragout = "",
                      TextureResType texType = TextureResType::LOCAL);

    /**
     * Load idle state texture for button.
     *
     * @param idle    idle state texture.
     * @param texType    @see `TextureResType`
     */
    void loadTextureIdle(const std::string& idle, TextureResType texType = TextureResType::LOCAL);

    /**
     * Load selected state texture for button.
     *
     * @param selected    selected state texture.
     * @param texType    @see `TextureResType`
     */
    void loadTexturePushed(const std::string& selected, TextureResType texType = TextureResType::LOCAL);

    /**
     * Load dragout state texture for button.
     *
     * @param dragout    dark state texture.
     * @param texType    @see `TextureResType`
     */
    void loadTextureDragout(const std::string& dragout, TextureResType texType = TextureResType::LOCAL);

    /**
     * Sets capInsets for button.
     * The capInset affects  all button scale9 renderer only if `setScale9Enabled(true)` is called
     *
     * @param capInsets    capInset in Rect.
     */
    void setCapInsets(const cocos2d::Rect &capInsets);

    /**
     * Sets capInsets for button, only the idle state scale9 renderer will be affected.
     *
     * @param capInsets    capInsets in Rect.
     */
    void setCapInsetsIdleRenderer(const cocos2d::Rect &capInsets);

    /**
     * Return the capInsets of idle state scale9sprite.
     *@return The idle scale9 renderer capInsets.
     */
    const cocos2d::Rect& getCapInsetsIdleRenderer()const;

    /**
     * Sets capInsets for button, only the pushed state scale9 renderer will be affected.
     *
     * @param capInsets    capInsets in Rect
     */
    void setCapInsetsPushedRenderer(const cocos2d::Rect &capInsets);

    /**
     * Return the capInsets of pushed state scale9sprite.
     *@return The pushed scale9 renderer capInsets.
     */
    const cocos2d::Rect& getCapInsetsPushedRenderer()const;

    /**
     * Sets capInsets for button, only the dragout state scale9 renderer will be affected.
     *
     * @param capInsets  capInsets in Rect.
     */
    void setCapInsetsDragoutRenderer(const cocos2d::Rect &capInsets);

    /**
     * Return the capInsets of dragout state scale9sprite.
     *@return The dragout scale9 renderer capInsets.
     */
    const cocos2d::Rect& getCapInsetsDragoutRenderer()const;

    /**
     * Enable scale9 renderer.
     *
     * @param enable Set to true will use scale9 renderer, false otherwise.
     */
    virtual void setScale9Enabled(bool enable);

    /**
     * Query whether button is using scale9 renderer or not.
     *@return whether button use scale9 renderer or not.
     */
    bool isScale9Enabled()const;

    //override methods
    virtual void ignoreContentAdaptWithSize(bool ignore) override;
    virtual cocos2d::Size getVirtualRendererSize() const override;
    virtual Node* getVirtualRenderer() override;
    virtual std::string getDescription() const override;

	void addButtonChild(cocos2d::Node* child);
	void addButtonChild(cocos2d::Node* child, ButtonState);
	void removeButtonChild(cocos2d::Node* child);

    /** @brief When user pushed the button, the button will zoom to a scale.
     * The final scale of the button  equals (button original scale + _zoomScale)
     * @since v3.3
     */
    void setZoomScale(float scale);

    /**
     * @brief Return a zoom scale
     * @return the zoom scale in float
     * @since v3.3
     */
    float getZoomScale()const;
    
    /**
     * @brief Return the nine-patch sprite of idle state
     * @return the nine-patch sprite of idle state
     * @since v3.9
     */
	cocos2d::ui::Scale9Sprite* getRendererIdle() const { return _buttonIdleRenderer; }
    
    /**
     * @brief Return the nine-patch sprite of pushed state
     * @return the nine-patch sprite of pushed state
     * @since v3.9
     */
	cocos2d::ui::Scale9Sprite* getRendererPushed() const { return _buttonPushedRenderer; }
    
    /**
     * @brief Return the nine-patch sprite of dragout state
     * @return the nine-patch sprite of dragout state
     * @since v3.9
     */
	cocos2d::ui::Scale9Sprite* getRendererDragout() const { return _buttonDragoutRenderer; }

    void resetIdleRender();
    void resetPushedRender();
    void resetDragoutRender();

    cocos2d::ResourceData getIdleFile();
	cocos2d::ResourceData getPushedFile();
	cocos2d::ResourceData getDragoutFile();

	cocos2d::Size getExpandZone() const;
	void setExpandZone(const cocos2d::Size& expandZone);
	cocos2d::Rect getExpandZoneRect() const;

	cocos2d::Size getSafeZone() const;
	void setSafeZone(const cocos2d::Size& safeZone);
	cocos2d::Rect getSafeZoneRect() const;

	typedef std::function<void(cocos2d::EventCustom*)> TouchEndedCallback;
	struct TouchEndedCallbackData
	{
		MyButton* button;
		MyButton::ButtonState state;
		bool is_long;
	};
	void setTouchEndedCallback(const TouchEndedCallback&);

	float getPushedTimeout();
	void setPushedTimeout(float);

CC_CONSTRUCTOR_ACCESS:
    virtual bool init() override;
    virtual bool init(const std::string& idleImage,
                      const std::string& selectedImage = "",
                      const std::string& disableImage = "",
                      TextureResType texType = TextureResType::LOCAL);

    virtual cocos2d::Size getIdleTextureSize() const;

protected:
    virtual void initRenderer() override;
    virtual void onSizeChanged() override;
	virtual void update(float dt) override;

    void loadTextureIdle(cocos2d::SpriteFrame* idleSpriteFrame);
    void setupIdleTexture(bool textureLoaded);
    void loadTexturePushed(cocos2d::SpriteFrame* pushedSpriteFrame);
    void setupPushedTexture(bool textureLoaded);
    void loadTextureDragout(cocos2d::SpriteFrame* dragoutSpriteFrame);
    void setupDragoutTexture(bool textureLoaded);

    void idleTextureScaleChangedWithSize();
    void pushedTextureScaleChangedWithSize();
    void dragoutTextureScaleChangedWithSize();

    virtual void adaptRenderers() override;
    void updateContentSize();

    virtual cocos2d::ui::Widget* createCloneInstance() override;
    virtual void copySpecialProperties(Widget* model) override;

    virtual cocos2d::Size getIdleSize() const;

	bool hitTest(const cocos2d::Vec2 &pt,
		const cocos2d::Camera* camera, cocos2d::Vec3 *p) const override;
	bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unusedEvent) override;
	void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unusedEvent) override;
	void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unusedEvent) override;
	void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unusedEvent) override;

protected:
	ButtonState _buttonState;

	cocos2d::ui::Scale9Sprite* _buttonIdleRenderer;
	cocos2d::ui::Scale9Sprite* _buttonPushedRenderer;
	cocos2d::ui::Scale9Sprite* _buttonDragoutRenderer;

	struct ChildInfo
	{
		cocos2d::Node* node;
		ButtonState state;
		bool state_any;
	};
	std::list<ChildInfo> _childNodes;

    float _zoomScale;
    bool _prevIgnoreSize;
    bool _scale9Enabled;

    cocos2d::Rect _capInsetsIdle;
	cocos2d::Rect _capInsetsPushed;
	cocos2d::Rect _capInsetsDragout;

	cocos2d::Size _idleTextureSize;
	cocos2d::Size _pushedTextureSize;
	cocos2d::Size _dragoutTextureSize;

    bool _idleTextureLoaded;
    bool _pushedTextureLoaded;
    bool _dragoutTextureLoaded;
    bool _idleTextureAdaptDirty;
    bool _pushedTextureAdaptDirty;
    bool _dragoutTextureAdaptDirty;

    std::string _idleFileName;
    std::string _pushedFileName;
    std::string _dragoutFileName;
    TextureResType _idleTexType;
    TextureResType _pushedTexType;
    TextureResType _dragoutTexType;

	cocos2d::Size _expandZone;
	cocos2d::Size _safeZone;

	TouchEndedCallback _touchEndedCallback;
	float _pushedTimeout;
	float _pushedTime;

private:
	cocos2d::Rect _makeZoneRect(const cocos2d::Size&) const;
	void _refreshButtonState(const cocos2d::Vec2*);
	void _generateEvent(ButtonState, bool is_long);
	void _updateChildren();
};

#endif
