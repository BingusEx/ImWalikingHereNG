#pragma once

namespace Events {
    class MenuOpenCloseHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        using EventResult = RE::BSEventNotifyControl;

        [[nodiscard]] static auto GetSingleton() -> util::not_null<MenuOpenCloseHandler*> {
            static MenuOpenCloseHandler singleton;
            return &singleton;
        }

        virtual EventResult ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                         RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
            const auto intfcStr = RE::InterfaceStrings::GetSingleton();
            if (a_event->menuName == intfcStr->dialogueMenu) {
                _inDialogue = a_event->opening;
            }

            return EventResult::kContinue;
        }

        void Install() {
            const auto ui = RE::UI::GetSingleton();
            ui->AddEventSink(MenuOpenCloseHandler::GetSingleton());
            logger::debug("Added menu open/close event sink"sv);
        }

        bool IsInDialogue() const { return _inDialogue; }

    private:
        MenuOpenCloseHandler() = default;
        MenuOpenCloseHandler(const volatile MenuOpenCloseHandler&) = delete;
        MenuOpenCloseHandler& operator=(const volatile MenuOpenCloseHandler&&) = delete;

        std::atomic_bool _inDialogue;
    };

    inline void Install() {
        MenuOpenCloseHandler::GetSingleton()->Install();
        logger::debug("Installed all event handlers");
    }
}  // namespace Events