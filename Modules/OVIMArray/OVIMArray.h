#ifndef _OVIMARRAY_H
#define _OVIMARRAY_H

#include <OpenVanilla/OpenVanilla.h>
#include "ArrayKeySequence.h"
#include "OVCandidateList.h"
#include "OVCIN.h"

class OVIMArray;
class OVIMArrayContext : public OVInputMethodContext
{
private:
    OVIMArray* parent;
    OVCIN* cintab;
    ArrayKeySequence keyseq;
    OVCandidateList candi_list;
public:
    OVIMArrayContext(OVIMArray* p, OVCIN* tab) 
        : parent(p), cintab(tab), keyseq(tab) { 
    }
    virtual int keyEvent(OVKeyCode* , OVBuffer* , OVCandidate* , OVService* );
    virtual void updateDisplay(OVBuffer*);
    int compose(OVBuffer *buf, OVCandidate *textbar, OVService *srv);
};

class OVIMArray : public OVInputMethod
{
public:
    virtual int initialize(OVDictionary *, OVService*, const char *mp);
    virtual const char* identifier() { return "OVIMArray"; }
    virtual OVInputMethodContext *newContext() { 
        return new OVIMArrayContext(this,cintab); 
    }
private:
    OVCIN* cintab;
};

#endif
