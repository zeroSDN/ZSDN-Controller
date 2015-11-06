/*
 * @date Created on [TODO-DATE].
 * @author [TODO-AUTHOR]
 */

#include "TemplateModule.hpp"


TemplateModule::TemplateModule(uint64_t instanceId) :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId), MODULE_VERSION, "TemplateModule",
                       std::vector<zmf::ModuleDependency>()) { }

TemplateModule::~TemplateModule() {
}


bool TemplateModule::enable() {
    // TODO Enable your module (eg. start threads, subscribe), return false if enabling fails
    return true;
}


void TemplateModule::disable() {
    // TODO Disable your module here (eg. stop threads)
}


void TemplateModule::handleModuleStateChange(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                             zmf::data::ModuleState newState, zmf::data::ModuleState lastState) {
    // Optional: Handle state changes of modules
}


zmf::data::ZmfOutReply TemplateModule::handleRequest(const zmf::data::ZmfMessage& message,
                                                     const zmf::data::ModuleUniqueId& sender) {
    // Optional: Correctly reply by returning a reply message
    return zmf::data::ZmfOutReply::createNoReply();
}
