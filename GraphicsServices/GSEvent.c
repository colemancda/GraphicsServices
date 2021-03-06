//
//  GSEvent.c
//  GraphicsServices
//
//  Created by Robert Widmann on 7/31/15.
//  Copyright © 2015 CodaFi. All rights reserved.
//

#include "GSEvent.h"
#include "GSCoreFoundationBridge.h"
#include "GSPrivate.h"

CFTypeRef _GSEventCopy(CFAllocatorRef, CFTypeRef);
CFStringRef _GSEventCopyDescription(CFTypeRef);
Boolean _GSEventEqualToEvent(CFTypeRef, CFTypeRef);

static CFTypeID kGSEventTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass _GSEventClass = {
	0,
	"GSEventClass",
	NULL,       // init
	_GSEventCopy, // copy
	NULL,		// dealloc
	_GSEventEqualToEvent, // isEqual
	NULL,		// hash
	NULL,       // formatting description
	_GSEventCopyDescription, // description
};


struct __GSEvent {
	CFRuntimeBase _base; // 0x0
	CGSEventType type; // 0x10
	//
	//
	//
	CGFloat windowLocationX; // 0x28
	CGFloat windowLocationY; // 0x30
	//
	CFAbsoluteTime timestamp; // 0x40
	CGWindowID windowID;
	CGEventRef cgEvent;
};

void _GSEventClassInitialize() {
	kGSEventTypeID = _CFRuntimeRegisterClass(&_GSEventClass);
}

CFTypeRef _GSEventCopy(CFAllocatorRef allocator, CFTypeRef cf) {
	GSEventRef copiedEvent = (GSEventRef)_CFRuntimeCreateInstance(NULL, kGSEventTypeID, /*cf->usagePage*/ 0, 0);
	if (copiedEvent != NULL) {
		memcpy(copiedEvent + sizeof(CFRuntimeBase), cf + sizeof(CFRuntimeBase), sizeof(struct __GSEvent) - sizeof(CFRuntimeBase));
	}
	return copiedEvent;
}

static const char *GSEventTypeTable[] = {
#define X(EVT, VAL, IDX) #EVT,
	GS_EVENT_TYPES_TABLE
#undef X
};

static const uint32_t GSEventIndexTable[][2] = {
#define X(EVT, VAL, IDX) { VAL, IDX },
	GS_EVENT_TYPES_TABLE
#undef X
};

static const char *CGSDescribeType(CGSEventType type) {
	for (uint32_t i = 0; i < kCGSEventCount; i++) {
		const uint32_t *tpp = GSEventIndexTable[i];
		if (tpp[0] == type) {
			return GSEventTypeTable[tpp[1]];
		}
	}
	return NULL;
}

CFStringRef _GSEventCopyDescription(CFTypeRef t) {
	struct __GSEvent *evt = (struct __GSEvent *)t;
	return CFStringCreateWithFormat(CFGetAllocator(t), NULL, CFSTR("<GSEvent %p>{type = %s, windowLoc = (%f, %f)}"), t, CGSDescribeType(evt->type), evt->windowLocationX, evt->windowLocationY);
}

Boolean _GSEventEqualToEvent(CFTypeRef cf1, CFTypeRef cf2) {
	if (cf1 == NULL || cf2 == NULL) {
		return false;
	}
	
	struct __GSEvent *evt1 = (struct __GSEvent *)cf1;
	struct __GSEvent *evt2 = (struct __GSEvent *)cf2;
	return (evt1->type == evt2->type) && (evt1->timestamp == evt2->timestamp) && (evt1->windowID == evt2->windowID);
}

CFTypeID GSEventGetTypeID() {
	return kGSEventTypeID;
}

GSEventRef GSEventCreateWithCGEvent(CFAllocatorRef allocator, CGEventRef event) {
	if (event == NULL) {
		return NULL;
	}

	uint32_t size = sizeof(struct __GSEvent) - sizeof(CFRuntimeBase);
	GSEventRef memory = (void *)_CFRuntimeCreateInstance(allocator, GSEventGetTypeID(), size, NULL);
	if (memory == NULL) {
		return NULL;
	}

	memory->cgEvent = (CGEventRef)CFRetain(event);
	memory->type = CGEventGetType(event);
	memory->windowID = (CGWindowID)CGEventGetIntegerValueField(event, kCGMouseEventWindowUnderMousePointer);
	CGPoint windowLoc = CGEventGetWindowLocation(event);
	memory->windowLocationX = windowLoc.x;
	memory->windowLocationY = windowLoc.y;
	return memory;
}

CGSEventType GSEventGetType(GSEventRef ref) {
	if (ref == NULL) {
		return -1;
	}
	return ref->type;
}

CGPoint GSEventGetLocationInWindow(GSEventRef ref) {
	if (ref == NULL) {
		return CGPointZero;
	}
	return (CGPoint){ ref->windowLocationX, ref->windowLocationY };
}

CFAbsoluteTime GSEventGetTimestamp(GSEventRef ref) {
	if (ref == NULL) {
		return 0;
	}
	return ref->timestamp;
}

CGWindowID GSEventGetWindowID(GSEventRef ref) {
	if (ref == NULL) {
		return 0;
	}
	return ref->windowID;
}

CGEventRef GSEventGetCGEvent(GSEventRef ref) {
	if (ref == NULL) {
		return 0;
	}
	return ref->cgEvent;
}
