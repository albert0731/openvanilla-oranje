// OVSCIM.cpp: the first OpenVanilla-SCIM bridge
#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_ICONV
#define Uses_SCIM_DEBUG
#define Uses_SCIM_C_STRING

#include <stdio.h>
#include "SCIMOV.h"
#include "OVPhoneticLib.cpp"
#include "OVIMTibetan-SCIM.cpp"
#include "OVIMPhoneticStatic-SCIM.cpp"

extern "C" {
    #include "OVPhoneticData.c"
};

using namespace scim;

static ConfigPointer _scim_config(0);

extern "C" void scim_module_init() {
}

extern "C" void scim_module_exit() {
    _scim_config.reset();
}

extern "C" unsigned int scim_imengine_module_init(const ConfigPointer& c) {
    _scim_config=c;
    return 2;
}

extern "C" IMEngineFactoryPointer scim_imengine_module_create_factory(uint32 e)
{
    // we have only one engine
    switch (e) {
        case 0: return new OVSCIMFactory(new OVIMPhoneticStatic, _scim_config);
        case 1: return new OVSCIMFactory(new OVIMTibetan, _scim_config);
    }
    
    return IMEngineFactoryPointer(0);
}

// OVSCIMFactory

OVSCIMFactory::OVSCIMFactory(OVInputMethod *i, const ConfigPointer& config) {
    fprintf(stderr, "SCIM-OpenVanilla IMFactory init! id=%s\n", i->identifier());
	set_languages("zh_TW,zh_HK,zh_SG");
	
	im=i;
	DummyDictionary dict;
	DummyService srv;
    im->initialize(&dict, &srv, "/tmp/");
}

OVSCIMFactory::~OVSCIMFactory() {
    delete im;
}

WideString OVSCIMFactory::get_name() const {
    char idbuf[256];
    sprintf(idbuf, "OpenVanilla-%s", im->localizedName("en"));
	return utf8_mbstowcs(idbuf);
}

String OVSCIMFactory::get_uuid() const {
    char hash[256];
    // we just generate some random stuff
    sprintf(hash, "d1f40e24-cdb7-11d9-9359-02061b%02x%02x%02x",
        strlen(im->identifier()), strlen(im->localizedName("en")),
        strlen(im->localizedName("zh_TW")));
	return String(hash);
}

String OVSCIMFactory::get_icon_file() const {
	return String("/usr/X11R6/share/scim/icons/rawcode.png");
}

WideString OVSCIMFactory::get_authors() const {
	return utf8_mbstowcs("The OpenVanilla Project <http://openvanilla.org>");
}

WideString OVSCIMFactory::get_credits() const {
	return WideString();
}

WideString OVSCIMFactory::get_help() const {
	return utf8_mbstowcs("Help unavailable");
}

IMEngineInstancePointer OVSCIMFactory::create_instance(const String& encoding, int id) {
    return new OVSCIMInstance(im->newContext(), this, encoding, id);
}


OVSCIMInstance::OVSCIMInstance(OVInputMethodContext *c, OVSCIMFactory *factory, const String& encoding, int id ) : DIMEInstance(factory,encoding, id), buf(this), candi(this)
{
    cxt=c;
    cxt->start(&buf, &candi, &srv);
}

OVSCIMInstance::~OVSCIMInstance() {
    delete cxt;
}

bool OVSCIMInstance::process_key_event(const KeyEvent& key) {
    // an OpenOffice workaround, code from SCIM-chewing
	if (key.is_key_release()) return true;

    DummyKeyCode kc;
    
    int c=key.get_ascii_code();

    if (key.mask & SCIM_KEY_ShiftMask) kc.setShift(1); 
    if (key.mask & SCIM_KEY_CapsLockMask) kc.setCapslock(1);
    if (key.mask & SCIM_KEY_ControlMask) kc.setCtrl(1);
    if (key.mask & SCIM_KEY_AltMask) kc.setAlt(1);
    
    switch (key.code) {
        case SCIM_KEY_Shift_L:
        case SCIM_KEY_Control_L:
        case SCIM_KEY_Alt_L:
        case SCIM_KEY_Left:      c=ovkLeft; break;
        case SCIM_KEY_Shift_R:
        case SCIM_KEY_Control_R:
        case SCIM_KEY_Alt_R:
        case SCIM_KEY_Right:     c=ovkRight; break;
        case SCIM_KEY_Up:        c=ovkUp; break;
        case SCIM_KEY_Down:      c=ovkDown; break;
        case SCIM_KEY_Delete:    c=ovkDelete; break;
        case SCIM_KEY_Home:      c=ovkHome; break;
        case SCIM_KEY_End:       c=ovkEnd; break;
        case SCIM_KEY_Tab:       c=ovkTab; break;            
        case SCIM_KEY_BackSpace: c=ovkBackspace; break;
        case SCIM_KEY_Escape:    c=ovkEsc; break;
        case SCIM_KEY_space:     c=ovkSpace; break;
        case SCIM_KEY_Return:    c=ovkReturn; break;
    }
    
    kc.setCode(c);
    if (!cxt->keyEvent(&kc, &buf, &candi, &srv)) return false;
	return true;
}

void OVSCIMInstance::select_candidate (unsigned int index) {
    // a candidate at index is choosen
}

void OVSCIMInstance::update_lookup_table_page_size(unsigned int page_size) {
}

void OVSCIMInstance::lookup_table_page_up() {
    // "page up" of candidate list
}

void OVSCIMInstance::lookup_table_page_down() {
    // "page down" of candidate list
}

void OVSCIMInstance::reset() {
    // reset everything, incl. context properties
    cxt->clear();
}

void OVSCIMInstance::focus_in() {
    // clear buffer, context::start()
    cxt->clear();
}

void OVSCIMInstance::focus_out() {
    // send out remaining texts
    cxt->clear();
}

