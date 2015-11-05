#ifndef TemplateModule_H
#define TemplateModule_H

#include <zmf/AbstractModule.hpp>

/**
 * TODO Description.
 * @date Created on [TODO-DATE].
 * @author [TODO-AUTHOR]
 */
class TemplateModule : public zmf::AbstractModule {

public:
    TemplateModule(uint64_t instanceId);

    ~TemplateModule();

    /**
     * Optional: Handle state changes of other modules
     */
    virtual void handleModuleStateChange(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                         zmf::data::ModuleState newState, zmf::data::ModuleState lastState) override;

    /**
     * Optional: Handle incoming ZMQ requests from other modules
     * @return True if we want to respond on the request, false otherwise
     */
    virtual zmf::data::ZmfOutReply handleRequest(const zmf::data::ZmfMessage& message,
                                                 const zmf::data::ModuleUniqueId& sender) override;


protected:
    /**
     * CALLED, BY CORE, DONT CALL IT FROM MODULE. MODULE CAN NOT INFLUENCE ENABLING
     * Called when the module should enable itself. Must initialize and start the module.
     * @return True if enable successful, False if enable failed or rejected
     */
    virtual bool enable();

    /**
     * CALLED, BY CORE, DONT CALL IT FROM MODULE. USE getZmf()->requestDisable
     * Called when the module should disable itself. Must stop the module (threads etc.).
     */
    virtual void disable();


private:
    static const uint16_t MODULE_VERSION = 0;
    static const uint16_t MODULE_TYPE_ID = 0x0000;

};


#endif // TemplateModule_H
