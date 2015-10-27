#ifndef DummyModule_H
#define DummyModule_H

#include <zmf/AbstractModule.hpp>

class DummyModule : public zmf::AbstractModule {

public:
    DummyModule(uint64_t fakeInstanceId,
                uint16_t fakeVersion,
                uint16_t fakeId,
                std::string fakeName,
                const std::function<void(
                        std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                        zmf::data::ModuleState lastState)>& changeStateCallback,

                const std::function<zmf::data::ZmfOutReply(
                        const zmf::data::ZmfMessage& message,
                        const zmf::data::ModuleUniqueId& sender)>& requestCallback)
            :
            AbstractModule(zmf::data::ModuleUniqueId(fakeId, fakeInstanceId), fakeVersion, fakeName, {}),
            changeStateCallback(changeStateCallback),
            requestCallback(requestCallback) { }

    ~DummyModule() { }

    inline zmf::IZmfInstanceAccess* const getZmfForUnittests() { return getZmf(); }

    virtual void handleModuleStateChange(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                         zmf::data::ModuleState newState,
                                         zmf::data::ModuleState lastState) override {
        changeStateCallback(changedModule, lastState);
    }

    virtual zmf::data::ZmfOutReply handleRequest(const zmf::data::ZmfMessage& message,
                                                 const zmf::data::ModuleUniqueId& sender) override {
        // Invalid response - no handler in derived class
        return requestCallback(message, sender);
    }


protected:
    virtual bool enable() { return true; }

public:
    virtual void disable() { }

    uint64_t fakeInstanceId;
    uint16_t fakeVersion;
    uint16_t fakeId;
    std::string fakeName;

    std::function<void(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                       zmf::data::ModuleState lastState)> changeStateCallback;

    std::function<zmf::data::ZmfOutReply(const zmf::data::ZmfMessage& message,
                                         const zmf::data::ModuleUniqueId& sender)> requestCallback;

};


#endif // DummyModule_H
