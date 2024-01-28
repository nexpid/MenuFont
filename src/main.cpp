#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>

#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/SettingNode.hpp>
#include <Geode/modify/CCLabelBMFont.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/GameManager.hpp>

#include "Conversion.hpp"

//                WARNING:
// you're reading the code of someone who has
// NEVER touched C++ in his entire life

using namespace geode::prelude;

struct SettingFontStruct {
    int m_fontIndex;
};

std::string useFont(int index = Mod::get()->getSettingValue<SettingFontStruct>("font").m_fontIndex) {
    return "gjFont" + std::string(index < 10 ? "0" : "") + std::to_string(index) + ".fnt";
};
float useFontScale(int index = Mod::get()->getSettingValue<SettingFontStruct>("font").m_fontIndex) {
    return fontMeasurements[index];
}

class SettingFontValue;
class SettingFontValue : public SettingValue {
   protected:
    int m_fontIndex;

   public:
    SettingFontValue(std::string const& key, std::string const& mod, int const& fontIndex)
        : SettingValue(key, mod), m_fontIndex(fontIndex) {}

    bool load(matjson::Value const& json) override {
        try {
            m_fontIndex = static_cast<int>(json.as<int>());
            return true;
        } catch (...) {
            return false;
        }
    }
    bool save(matjson::Value& json) const override {
        json = static_cast<int>(m_fontIndex);
        return true;
    }

    int getFont() const {
        return m_fontIndex;
    }
    void setFont(int font) {
        m_fontIndex = font;
    }

    SettingNode* createNode(float width) override;
};

template <>
struct SettingValueSetter<SettingFontStruct> {
    static SettingFontStruct get(SettingValue* setting) {
        auto fontSetting = static_cast<SettingFontValue*>(setting);
        struct SettingFontStruct defaultStruct = {fontSetting->getFont()};
        return defaultStruct;
    };
    static void set(SettingFontValue* setting, SettingFontStruct const& value) {
        setting->setFont(value.m_fontIndex);
    };
};

class SettingFontNode : public SettingNode {
   protected:
    int m_fontIndex;
    CCLabelBMFont* m_nameLabel;
    CCLabelBMFont* m_valueLabel;
    CCMenuItemSpriteExtra* m_resetBtn;

    bool init(SettingFontValue* value, float width) {
        if (!SettingNode::init(value))
            return false;

        m_fontIndex = value->getFont();

        auto menu = CCMenu::create();
        menu->setTouchEnabled(true);

        m_nameLabel = CCLabelBMFont::create("Font", "bigFont.fnt");
        m_nameLabel->setScale(.6F);
        m_nameLabel->setPositionX(-130);

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(.6F);

        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoSpr,
            this,
            menu_selector(SettingFontNode::onDescription));
        infoBtn->setPositionX(-95);

        auto resetSpr = CCSprite::create("reset-gold.png"_spr);
        resetSpr->setScale(.5F);

        m_resetBtn = CCMenuItemSpriteExtra::create(
            resetSpr,
            this,
            menu_selector(SettingFontNode::onReset));
        m_resetBtn->setPositionX(-75);

        auto leftArrSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        leftArrSpr->setScale(.6F);
        auto rightArrSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        rightArrSpr->setFlipX(true);
        rightArrSpr->setScale(.6F);

        auto leftArrBtn = CCMenuItemSpriteExtra::create(
            leftArrSpr,
            this,
            menu_selector(SettingFontNode::onSwapFont));
        auto rightArrBtn = CCMenuItemSpriteExtra::create(
            rightArrSpr,
            this,
            menu_selector(SettingFontNode::onSwapFont));

        leftArrBtn->setTag(-1);
        leftArrBtn->setPositionX(30);
        rightArrBtn->setTag(1);
        rightArrBtn->setPositionX(150);

        m_valueLabel = CCLabelBMFont::create(std::format("Font {}", m_fontIndex).c_str(), useFont(m_fontIndex).c_str());
        m_valueLabel->setScale(.6F * useFontScale(m_fontIndex));
        m_valueLabel->setPositionX(90);

        menu->setPosition(width / 2, 18.f);
        menu->addChild(m_nameLabel);
        menu->addChild(infoBtn);
        menu->addChild(m_resetBtn);
        menu->addChild(m_valueLabel);
        menu->addChild(leftArrBtn);
        menu->addChild(rightArrBtn);
        this->addChild(menu);

        this->setContentSize({width, 40.f});

        return true;
    }

    void onSwapFont(CCObject* sender) {
        if (sender->getTag() == -1) {
            m_fontIndex = m_fontIndex == 1 ? 59 : m_fontIndex - 1;
        } else if (sender->getTag() == 1) {
            m_fontIndex = (m_fontIndex % 59) + 1;
        }
        this->valueChanged();
    }

    void onDescription(CCObject* sender) {
        FLAlertLayer::create(
            "Font",
            "Choose a font from the available <cy>custom level fonts</c> and use it as the <cl>font in the menu</c>.",
            "OK"
        )->show();
    }

    void onReset(CCObject*) {
        createQuickPopup(
            "Reset",
            "Are you sure you want to <cr>reset</c> <cl>Font</c> to <cy>default</c>?",
            "Cancel", "Reset",
            [this](auto, bool btn2) {
                if (btn2) {
                    this->resetToDefault();
                }
            });
    }

   public:
    void valueChanged() {
        m_nameLabel->setColor(
            this->hasUncommittedChanges()
                ? cc3x(0x1d0)
                : cc3x(0xfff));

        m_resetBtn->setVisible(this->hasNonDefaultValue());
        
        m_valueLabel->setString(std::format("Font {}", m_fontIndex).c_str());
        m_valueLabel->setScale(.6F * useFontScale(m_fontIndex));
        m_valueLabel->setFntFile(useFont(m_fontIndex).c_str());

        this->dispatchChanged();
    }

    void commit() override {
        static_cast<SettingFontValue*>(m_value)->setFont(m_fontIndex);
        this->valueChanged();
        this->dispatchCommitted();
    }
    bool hasUncommittedChanges() override {
        return m_fontIndex != static_cast<SettingFontValue*>(m_value)->getFont();
    }
    bool hasNonDefaultValue() override {
        return m_fontIndex != 1;
    }
    void resetToDefault() override {
        m_fontIndex = 1;
        this->valueChanged();
    }

    static SettingFontNode* create(SettingFontValue* value, float width) {
        auto ret = new SettingFontNode;
        if (ret && ret->init(value, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

SettingNode* SettingFontValue::createNode(float width) {
    return SettingFontNode::create(this, width);
}

$on_mod(Loaded) {
    Mod::get()->addCustomSetting<SettingFontValue>("font", 1);
}

void toCMYK(float red, float green, float blue, float* cmyk) {
    float k = std::min(255 - red, std::min(255 - green, 255 - blue));
    float c = 255 * (255 - red - k) / (255 - k);
    float m = 255 * (255 - green - k) / (255 - k);
    float y = 255 * (255 - blue - k) / (255 - k);

    cmyk[0] = c;
    cmyk[1] = m;
    cmyk[2] = y;
    cmyk[3] = k;
}

void toRGB(float c, float m, float y, float k, float* rgb) {
    rgb[0] = -((c * (255 - k)) / 255 + k - 255);
    rgb[1] = -((m * (255 - k)) / 255 + k - 255);
    rgb[2] = -((y * (255 - k)) / 255 + k - 255);
}

bool shouldDisable() {
    auto manager = GameManager::get();

    auto playLayer = manager->getPlayLayer();
    auto editorLayer = manager->getEditorLayer();

    return playLayer != nullptr || editorLayer != nullptr;
}

class $modify(CCLabelBMFont) {
    cocos2d::ccColor3B color;
    float scale;
    bool disabled;

    bool initWithString(char const* str, char const* fntFile, float width, cocos2d::CCTextAlignment alignment, cocos2d::CCPoint imageOffset) {
        m_fields->color = fntFile == std::string("goldFont.fnt") ? ccc3(253, 196, 44) : ccc3(255, 255, 255);
        // having issues with chatFont scaling :(
        m_fields->scale = fntFile == std::string("chatFont.fnt") ? useFontScale() * chatFontRatio : useFontScale();

        m_fields->disabled = false;
        if (
            std::string(fntFile).starts_with("gjFont") ||
            fntFile == std::string("chatFont.fnt") ||
            shouldDisable()
        )
            m_fields->disabled = true;

        if (!CCLabelBMFont::initWithString(
            str,
            m_fields->disabled ? fntFile : useFont().c_str(),
            width,
            alignment,
            imageOffset
        )) return false;

        if (!m_fields->disabled) {
            setColor(ccc3(255, 255, 255));
            setScaleX(1);
            setScaleY(1);
        }

        return true;
    }

    void setColor(cocos2d::ccColor3B const& color) {
        if (m_fields->disabled)
            return CCLabelBMFont::setColor(color);

        float cmykBase[4];
        toCMYK(
            m_fields->color.r,
            m_fields->color.g,
            m_fields->color.b,
            cmykBase
        );

        float cmykNew[4];
        toCMYK(color.r, color.g, color.b, cmykNew);

        float cmykMix[] = {
            cmykBase[0] + cmykNew[0],
            cmykBase[1] + cmykNew[1],
            cmykBase[2] + cmykNew[2],
            cmykBase[3] + cmykNew[3]
        };

        float rgbFinal[3];
        toRGB(cmykMix[0], cmykMix[1], cmykMix[2], cmykMix[3], rgbFinal);

        CCLabelBMFont::setColor(ccc3(rgbFinal[0], rgbFinal[1], rgbFinal[2]));
    }

    void setScaleX(float scaleX) {
        CCLabelBMFont::setScaleX(m_fields->disabled ? scaleX : scaleX * m_fields->scale);
    }
    void setScaleY(float scaleY) {
        CCLabelBMFont::setScaleY(m_fields->disabled ? scaleY : scaleY * m_fields->scale);
    }
    void setScale(float scale) {
        CCLabelBMFont::setScale(m_fields->disabled ? scale : scale * m_fields->scale);
    }
};