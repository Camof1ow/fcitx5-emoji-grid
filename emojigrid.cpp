// fcitx5 애드온: Super+; 로 이모지 그리드 창(emoji-grid-ui)을 열고 닫고,
// 창에서 고른 이모지를 D-Bus Commit 으로 받아 입력기 경로(commitString)로 넣는다.
// 그리드 창은 키보드를 가져가지 않으므로 입력칸 포커스가 그대로 남는다.
#include <dbus_public.h>
#include <fcitx-utils/dbus/bus.h>
#include <fcitx-utils/dbus/objectvtable.h>
#include <fcitx-utils/key.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/instance.h>
#include <memory>
#include <string>

namespace {

class EmojiGrid;

class EmojiGridService : public fcitx::dbus::ObjectVTable<EmojiGridService> {
public:
    explicit EmojiGridService(EmojiGrid *parent) : parent_(parent) {}

    void commit(const std::string &text);
    void hide();
    bool isOpen();

    FCITX_OBJECT_VTABLE_SIGNAL(visibilityChanged, "VisibilityChanged", "b");

private:
    EmojiGrid *parent_;
    FCITX_OBJECT_VTABLE_METHOD(commit, "Commit", "s", "");
    FCITX_OBJECT_VTABLE_METHOD(hide, "Hide", "", "");
    FCITX_OBJECT_VTABLE_METHOD(isOpen, "IsOpen", "", "b");
};

class EmojiGrid : public fcitx::AddonInstance {
    // auto 반환 함수라 쓰기 전에 정의돼야 한다.
    FCITX_ADDON_DEPENDENCY_LOADER(dbus, instance_->addonManager());

public:
    explicit EmojiGrid(fcitx::Instance *instance)
        : instance_(instance), service_(this) {
        dbus()->call<fcitx::IDBusModule::bus>()->addObjectVTable(
            "/emojigrid", "org.fcitx.Fcitx.EmojiGrid1", service_);

        keyWatcher_ = instance_->watchEvent(
            fcitx::EventType::InputContextKeyEvent,
            fcitx::EventWatcherPhase::PreInputMethod,
            [this](fcitx::Event &event) {
                auto &keyEvent = static_cast<fcitx::KeyEvent &>(event);
                if (keyEvent.isRelease()) {
                    return;
                }
                if (keyEvent.key().check(trigger_)) {
                    setOpen(!open_);
                    keyEvent.filterAndAccept();
                } else if (open_ &&
                           keyEvent.key().check(FcitxKey_Escape)) {
                    setOpen(false);
                    keyEvent.filterAndAccept();
                }
            });
    }

    void commit(const std::string &text) {
        auto *ic = instance_->mostRecentInputContext();
        if (!ic || text.empty()) {
            return;
        }
        ic->commitString(text);
    }

    void setOpen(bool open) {
        if (open_ == open) {
            return;
        }
        open_ = open;
        service_.visibilityChanged(open_);
    }

    bool isOpen() const { return open_; }

private:
    fcitx::Instance *instance_;
    EmojiGridService service_;
    const fcitx::Key trigger_{FcitxKey_semicolon, fcitx::KeyState::Super};
    bool open_ = false;
    std::unique_ptr<fcitx::HandlerTableEntry<fcitx::EventHandler>> keyWatcher_;
};

void EmojiGridService::commit(const std::string &text) { parent_->commit(text); }
void EmojiGridService::hide() { parent_->setOpen(false); }
bool EmojiGridService::isOpen() { return parent_->isOpen(); }

class EmojiGridFactory : public fcitx::AddonFactory {
    fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
        return new EmojiGrid(manager->instance());
    }
};

} // namespace

FCITX_ADDON_FACTORY_V2(emojigrid, EmojiGridFactory);
