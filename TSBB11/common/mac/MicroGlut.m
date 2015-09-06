// Micro-GLUT, bare essentials
// Single-file GLUT subset.
// By Ingemar Ragnemalm 2012-2013

// I wrote this since GLUT seems not to have been updated to support
// creation of a 3.2 context on the Mac. With a single-file solution like
// this, there is no installation problems at all and it is also easy
// to hack around in a local copy as needed.

// To do: Linux/Windows/iOS ports. (Preliminary versions exist of both.)
// Some additional features. Text rendering? Menus? (OK) Multi-window?

// Changes:
// Several additions for the lab 3 version: Simple menu, glutTimerFunc and my own glutrepeatingTimerFunc.
// glutKeyboardUpFunc, glutInitDisplayMode.
// 120209: Some more additions, like glutMotionFunc and GLUT_RIGHT_BUTTON.
// 120228: Added glutSetWindowTitle, glutKeyIsDown, glutInitContextVersion
// NOTE: glutInitContextVersion is now required for 3.2 code!
// NOTE: This statement is incorrect; MicroGLUT still only supports 3.2 and with no extra calls.
// Note however the GL3ONLY define.
// 120301: Resizable window. Correct vertical window position.
// 120808: Fixed a bug that caused error messages. (Calling finishLaunching twice.)
// 120822: Stencil now uses 8 instead of 32
// 120905: Two corrections suggested by Marcus Stenbäck.
// 120913: Added 2-button emulation with CTRL
// 130103: Added [m_context makeCurrentContext]; in all user callbacks. (GL calls had no effect.)
// 130127: Added basic popup menu support
// 130214: Added glutSpecialFunc and glutSpecialUpFunc. Sets focus to window when created.
// 130220: Modified idle support, added fake visibility support.
// 130325: Corrected name of glutWarpPointer
// 130330: Added GLUT_MULTISAMPLE
// 130331: Added glutChangeMenuEntry
// 130513: Added GLUT_WINDOW_WIDTH and GLUT_WINDOW_HEIGHT support to glutGet (which is still lacking a whole bunch of other types). Modified glutWarpPointer.
// 130907: Added isFlipped to make mouse coordinates reasonable (and conform with Linux version).

// Incompatibility in mouse coordinates; global or local?
// Should be local!

#import <Cocoa/Cocoa.h>
#include <OpenGL/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/time.h>

#include "MicroGlut.h"

// Comment out to support GL2, requiring glutInitContextVersion for newer.
//#define GL3ONLY

// Vital internal variables

void (*gDisplay)(void);
void (*gReshape)(int width, int height);
void (*gKey)(unsigned char key, int x, int y);
void (*gSpecialKey)(unsigned char key, int x, int y);
void (*gKeyUp)(unsigned char key, int x, int y);
void (*gSpecialKeyUp)(unsigned char key, int x, int y);
void (*gMouseMoved)(int x, int y);
void (*gMouseDragged)(int x, int y);
void (*gMouseFunc)(int button, int state, int x, int y);
unsigned int gContextInitMode = GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH;
void (*gIdle)(void);
char updatePending = 1;
char gRunning = 1;
char gKeymap[256];
int gContextVersionMajor = 0;
int gContextVersionMinor = 0;

// -----------

// Globals (was in GLViewDataPtr)
NSOpenGLContext	*m_context;
float lastWidth, lastHeight;
NSView *theView;

void MakeContext(NSView *view)
{
	NSOpenGLPixelFormat *fmt;
	int zdepth, sdepth, i;
	int profile = 0, profileVersion = 0;
	
	if (gContextInitMode & GLUT_DEPTH)
		zdepth = 32;
	else
		zdepth = 0;
	
	if (gContextInitMode & GLUT_STENCIL)
		sdepth = 8;
	else
		sdepth = 0;

	NSOpenGLPixelFormatAttribute attrs[] =
	{
//		NSOpenGLPFAPixelBuffer,
//		NSOpenGLPFAAccelerated,
//		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, zdepth,
		NSOpenGLPFAStencilSize, sdepth,
		0,0,0,0,0,0,0
//		profile, profileVersion,
//		multi,
	};
	
	i = 4;
	
// Def this out about for disabling profile selection support,
// if it is on we are defaulting to GL 1/2
#ifndef GL3ONLY
	if (gContextVersionMajor == 3)
#endif
	// I ignore the minor version for now, 3.2 is all we can choose currently
	{
		profile = NSOpenGLPFAOpenGLProfile;
		profileVersion = NSOpenGLProfileVersion3_2Core;
		attrs[i++] = profile;
		attrs[i++] = profileVersion;
	}

	if (gContextInitMode & GLUT_DOUBLE)
	{
		attrs[i++] = NSOpenGLPFADoubleBuffer;
	}
	
	if (gContextInitMode & GLUT_MULTISAMPLE)
	{
		attrs[i++] = NSOpenGLPFAMultisample;
	}
	
	// Save view (should be packaged with context for multi-window application - to do)
	theView = view;
	
	// Init GL context
	fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: &attrs[0]];
	
	m_context = [[NSOpenGLContext alloc] initWithFormat: fmt shareContext: nil];
	[fmt release];
	[m_context makeCurrentContext];
}

// ---------------------- Menus ----------------------

// MicroGlut menu support
// This is a subset of the GLUT menu support, focused on what seems to be used
// in current GLUT-using demos. If nobody uses it, don't add it (unless it is
// a feature that you really want people to learn using).

typedef void (*MenuProc)(int value);
@interface TPopupMenu : NSMenu {@public MenuProc funcCallback;}
-(void)selectMenuItem:(id *)sender;
@end

@implementation TPopupMenu

-(void)selectMenuItem:(id *)sender
{
	funcCallback( [((NSMenuItem *) sender) tag]);
}

@end

TPopupMenu *currentMenu;

TPopupMenu *buttons[10]; // Menu by button id - for a button, what menu should be used, if any?
TPopupMenu *menuList[10]; // List of menus by (internal) menu id
int menuCount = -1;

int glutCreateMenu(void (*func)(int value))
{
	currentMenu = [[TPopupMenu alloc] initWithTitle: @""];
	currentMenu->funcCallback = func;
	menuCount += 1;
	menuList[menuCount] = currentMenu;
	return menuCount;
}
void glutAddMenuEntry(char *name, int value)
{
	NSMenuItem *menuItem1;
	
	menuItem1 = NSMenuItem.alloc;
	// string to NSString
	NSString * s = [NSString stringWithCString: name encoding: NSASCIIStringEncoding];
	[menuItem1 initWithTitle: s action: nil keyEquivalent: @""];
	[menuItem1 setTarget: currentMenu]; // Who will handle it?
	[menuItem1 setAction: @selector(selectMenuItem:)];
	[menuItem1 setTag: value]; // Save value
	[currentMenu addItem: menuItem1];
}

void glutChangeToMenuEntry(int index, char *name, int value)
{
	NSMenuItem *menuItem1;
	
	menuItem1 = [currentMenu itemAtIndex: index-1];
	if (menuItem1 != NULL)
	{
		// string to NSString
		NSString * s = [NSString stringWithCString: name encoding: NSASCIIStringEncoding];
		[menuItem1 setTarget: currentMenu]; // Who will handle it?
		[menuItem1 setAction: @selector(selectMenuItem:)];
		[menuItem1 setTag: value]; // Save value
		[menuItem1 setTitle: s];
	}
}



void glutAttachMenu(int button)
{
	// set a data item for the button
	buttons[button] = currentMenu;
}

void glutAddSubMenu(char *name, int menu)
{
	NSMenuItem *item;
	NSString * s = [NSString stringWithCString: name encoding: NSASCIIStringEncoding];
	
	item = [currentMenu addItemWithTitle: s action: nil keyEquivalent: @""];
	[currentMenu setSubmenu: menuList[menu] forItem: item];
}

void glutDetachMenu(int button)
{
	// reset a data item for the button
	buttons[button] = nil;	
}

// Check name on this!
void glutSetMenu(int menu)
{
	currentMenu = menuList[menu];
}

int glutGetMenu(void)
{
	int i;
	for (i = 0; i <= menuCount; i++)
		if (menuList[i] == currentMenu)
			return i;
	return -1;
}

// End of MicroGlut button support


static char doKeyboardEvent(NSEvent *theEvent, void (*func)(unsigned char key, int x, int y), void (*specialfunc)(unsigned char key, int x, int y), int keyMapValue)
{
	char *chars;
	
	chars = (char *)[[theEvent characters] cStringUsingEncoding: NSMacOSRomanStringEncoding];
	
	if (chars != NULL)
	{
		if (func != NULL) // Change 120913
			func(chars[0], 0, 0); // TO DO: x and y
		
		gKeymap[(unsigned int)chars[0]] = keyMapValue;
	}
	else
	{
		char code;
		switch( [theEvent keyCode] )
		{
			case 126: code = GLUT_KEY_UP; break;
			case 125: code = GLUT_KEY_DOWN; break;
			case 124: code = GLUT_KEY_RIGHT; break;
			case 123: code = GLUT_KEY_LEFT; break;
			case 122: code = GLUT_KEY_F1; break;
			case 120: code = GLUT_KEY_F2; break;
			case 99: code = GLUT_KEY_F3; break;
			case 118: code = GLUT_KEY_F4; break;
			case 96: code = GLUT_KEY_F5; break;
			case 97: code = GLUT_KEY_F6; break;
			case 98: code = GLUT_KEY_F7; break;
			case 115: code = GLUT_KEY_HOME; break; // ?
			case 116: code = GLUT_KEY_PAGE_UP; break;
			case 119: code = GLUT_KEY_END; break; // ?
			case 121: code = GLUT_KEY_PAGE_DOWN; break;
			case 117: code = GLUT_KEY_INSERT; break; // Looks more like DEL?
			default: code = [theEvent keyCode];
		}
		if (specialfunc != NULL) // Change 130114
			specialfunc(code, 0, 0); // TO DO: x and y
		gKeymap[code] = keyMapValue;
	}
}


// -------------------- View ------------------------

@interface TestView : NSView <NSWindowDelegate> { }
-(void)drawRect:(NSRect)rect;
-(void)keyDown:(NSEvent *)theEvent;
-(void)keyUp:(NSEvent *)theEvent;
-(void)mouseMoved:(NSEvent *)theEvent;
-(void)mouseDragged:(NSEvent *)theEvent;
-(void)mouseDown:(NSEvent *)theEvent;
-(void)mouseUp:(NSEvent *)theEvent;
-(void)rightMouseDown:(NSEvent *)theEvent;
-(void)rightMouseUp:(NSEvent *)theEvent;
-(void)windowDidresize:(NSNotification *)note;
-(BOOL)isFlipped;
@end

#define Pi 3.1415

@implementation TestView

-(void) mouseMoved:(NSEvent *)theEvent
{
	NSPoint p;
	[m_context makeCurrentContext];
	
	if (gMouseMoved != nil)
	{
		p = [theEvent locationInWindow];
		p = [self convertPoint: p fromView: nil];
		gMouseMoved(p.x, p.y);
	}
}

-(void) mouseDragged:(NSEvent *)theEvent
{
	NSPoint p;
	[m_context makeCurrentContext];
	
	if (gMouseDragged != nil)
	{
		p = [theEvent locationInWindow];
		p = [self convertPoint: p fromView: nil];
		gMouseDragged((int)p.x, (int)p.y);
	}
}

// Clone of above, but necessary for supporting the alternative button. Thanks to Marcus Stenbäck!
-(void) rightMouseDragged:(NSEvent *)theEvent
{
	NSPoint p;
	[m_context makeCurrentContext];
	
	if (gMouseDragged != nil)
	{
		p = [theEvent locationInWindow];
		p = [self convertPoint: p fromView: nil];
		gMouseDragged((int)p.x, (int)p.y);
	}
}

// Remember if last press on the left (default) button was modified to
// a "right" with CTRL
char gLeftIsRight = 0;

-(void) mouseDown:(NSEvent *)theEvent
{
	NSPoint p;
	[m_context makeCurrentContext];
	
	if (gMouseFunc != nil)
	{
		// Convert location in window to location in view
		p = [theEvent locationInWindow];
	printf("mouseDown before convertPoint %f %f \n", p.x, p.y);
		p = [self convertPoint: p fromView: nil];
	printf("mouseDown %f %f \n", p.x, p.y);
		
		if ([NSEvent modifierFlags] & NSControlKeyMask)
		{
			gMouseFunc(GLUT_RIGHT_BUTTON, GLUT_DOWN, p.x, p.y);
			gLeftIsRight = 1;
		}
		else
		{
			gMouseFunc(GLUT_LEFT_BUTTON, GLUT_DOWN, p.x, p.y);
			gLeftIsRight = 0;
		}
	}
//	else

	if ([NSEvent modifierFlags] & NSControlKeyMask)
	{
		if (buttons[GLUT_RIGHT_BUTTON] != nil)
			[NSMenu popUpContextMenu: buttons[GLUT_RIGHT_BUTTON]
						withEvent: theEvent forView: (NSButton *)self];
	}
	else
	{
		if (buttons[GLUT_LEFT_BUTTON] != nil)
			[NSMenu popUpContextMenu: buttons[GLUT_LEFT_BUTTON]
						withEvent: theEvent forView: (NSButton *)self];
	}
}

-(void) mouseUp:(NSEvent *)theEvent
{
	NSPoint p;
	[m_context makeCurrentContext];
	
	if (gMouseFunc != nil)
	{
		// Convert location in window to location in view
		p = [theEvent locationInWindow];
		p = [self convertPoint: p fromView: nil];
		
		// Assuming that the user won't release CTRL - then it looks like different buttons
		if (gLeftIsRight)
			gMouseFunc(GLUT_RIGHT_BUTTON, GLUT_UP, p.x, p.y);
		else
			gMouseFunc(GLUT_LEFT_BUTTON, GLUT_UP, p.x, p.y);
	}
}

-(void) rightMouseDown:(NSEvent *)theEvent
{
	NSPoint p;
	[m_context makeCurrentContext];
	
	if (gMouseFunc != nil)
	{
		// Convert location in window to location in view
		p = [theEvent locationInWindow];
		p = [self convertPoint: p fromView: nil];
		gMouseFunc(GLUT_RIGHT_BUTTON, GLUT_DOWN, p.x, p.y);
	}
	else
		if (buttons[GLUT_RIGHT_BUTTON] != nil)
			[NSMenu popUpContextMenu: buttons[GLUT_RIGHT_BUTTON]
						withEvent: theEvent forView: (NSButton *)self];
}

-(void) rightMouseUp:(NSEvent *)theEvent
{
	NSPoint p;
	[m_context makeCurrentContext];
	
	if (gMouseFunc != nil)
	{
		// Convert location in window to location in view
		p = [theEvent locationInWindow];
		p = [self convertPoint: p fromView: nil];
		gMouseFunc(GLUT_RIGHT_BUTTON, GLUT_UP, p.x, p.y);
	}
}

-(void)keyDown:(NSEvent *)theEvent
{
	char *chars;
	[m_context makeCurrentContext];
	doKeyboardEvent(theEvent, gKey, gSpecialKey, 1);
	
	/*
	// We only support ASCII. Why not UTF-8? Well, slightly more complicated, and is it needed?
	chars = (char *)[[theEvent characters] cStringUsingEncoding: NSMacOSRomanStringEncoding];
	if (chars != NULL)
	{
		if (chars[0] < 32 || chars[0] == 127)
		{
			if (gSpecialKey != NULL) // Change 130114
				gSpecialKey(chars[0], 0, 0); // TO DO: x and y
		}
		else
			if (gKey != NULL) // Change 120913
				gKey(chars[0], 0, 0); // TO DO: x and y
		
		gKeymap[(unsigned int)chars[0]] = 1;
	}
	*/
}

-(void)keyUp:(NSEvent *)theEvent
{
	char *chars;
	[m_context makeCurrentContext];
	doKeyboardEvent(theEvent, gKeyUp, gSpecialKeyUp, 0);
/*	
	chars = (char *)[[theEvent characters] cStringUsingEncoding: NSMacOSRomanStringEncoding];
	if (chars != NULL)
	{
		if (chars[0] < 32 || chars[0] == 127)
		{
			if (gSpecialKeyUp != NULL) // Change 130114
				gSpecialKeyUp(chars[0], 0, 0); // TO DO: x and y
		}
		else
			if (gKeyUp != NULL) // Change 120913
				gKeyUp(chars[0], 0, 0); // TO DO: x and y
//			printf("keyup %c\n", chars[0]);

		gKeymap[(unsigned int)chars[0]] = 0;
	}
*/
}

- (BOOL)acceptsFirstResponder	{ return YES; }

- (BOOL)becomeFirstResponder	{ return YES; }

- (BOOL)resignFirstResponder	{ return YES; }

-(void)drawRect:(NSRect)rect
{
	if (([theView frame].size.width != lastWidth) || ([theView frame].size.height != lastHeight))
	{
		lastWidth = [theView frame].size.width;
		lastHeight = [theView frame].size.height;
		
		// Only needed on resize:
		[m_context clearDrawable];
//		glViewport(0, 0, [theView frame].size.width, [theView frame].size.height);
		
		if (gReshape != NULL)
			gReshape([theView frame].size.width, [theView frame].size.height);
	}
	
	[m_context setView: theView];
	[m_context makeCurrentContext];
	
	// Draw
	updatePending = 0; // Did not help
	if (gDisplay != NULL)
		gDisplay();
	
	[m_context flushBuffer];
	[NSOpenGLContext clearCurrentContext];
}

-(void)windowWillClose:(NSNotification *)note
{
	[[NSApplication sharedApplication] terminate:self];
}

-(void)windowDidresize:(NSNotification *)note
{
	// Call gReshape now or in drawRect?
}

-(BOOL)isFlipped
{
	return YES;
}

@end


// -------------------- Timer ------------------------

// Data for timer
@interface TimerInfoRec : NSObject
{
@public	void (*func)(int arg);
	int arg;
@private
}
@end

@implementation TimerInfoRec
@end

// Mini-mini class for the timer
@interface TimerController : NSObject { }
-(void)timerFireMethod:(NSTimer *)t;
@end

NSTimer	*gTimer;
TimerController	*myTimerController;
NSView	*view;

// Timer!
@implementation TimerController
-(void)timerFireMethod:(NSTimer *)t;
{
	TimerInfoRec *tr;
	
	if (t.userInfo != nil) // One-shot timer with a TimerInfoRec
	{
		tr = t.userInfo;
		tr->func(tr->arg);
		[tr release];
//		((TimerInfoRec *)(t.userInfo))->func(((TimerInfoRec *)(t.userInfo))->arg);
//		free((TimerInfoRec *)(t.userInfo));
	}
	else
	{
		[view setNeedsDisplay: YES];
		updatePending = 1;
	}
}
@end


void glutPostRedisplay()
{
	[theView setNeedsDisplay: YES];
	updatePending = 1;
}


// home()

#include <Carbon/Carbon.h>
#include <stdio.h>

void home()
{
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	char path[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
	{
		// error!
		return;
	}
	CFRelease(resourcesURL);

	chdir(path);
	printf("Current Path: %s\n", path);
}

// ------------------ Main program ---------------------

@interface MGApplication : NSApplication
@end

@implementation MGApplication
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
	gRunning = 0;
}
@end

MGApplication *myApp;
NSView *view;
NSAutoreleasePool *pool;
NSWindow *window;
static struct timeval timeStart;

void CreateMenu()
{
	NSMenu *mainMenu, *theMiniMenu;
	NSMenuItem *menuItem2, *dummyItem;

	// Create main menu = menu bar
	mainMenu = NSMenu.alloc;
	[mainMenu initWithTitle: @""];
	[NSApp setMainMenu: mainMenu];

	// Create the custom menu
	theMiniMenu = NSMenu.alloc;
	[theMiniMenu initWithTitle: @"The MiniMenu"];
	
	// Create a menu item with standard message
	menuItem2 = NSMenuItem.alloc;
	[menuItem2 initWithTitle: @"Hide" action: @selector(hide:) keyEquivalent: @"h"];
	[menuItem2 setKeyEquivalentModifierMask: NSCommandKeyMask];
	[theMiniMenu addItem: menuItem2];
	
	// Create a menu item with standard message
	menuItem2 = NSMenuItem.alloc;
	[menuItem2 initWithTitle: @"Hide others" action: @selector(hideOtherApplications:) keyEquivalent: @"h"];
	[menuItem2 setKeyEquivalentModifierMask: NSCommandKeyMask | NSAlternateKeyMask];
	[theMiniMenu addItem: menuItem2];
	
	// Create a menu item with standard message
	menuItem2 = NSMenuItem.alloc;
	[menuItem2 initWithTitle: @"Show all" action: @selector(unhideAllApplications:) keyEquivalent: @"h"];
	[menuItem2 setKeyEquivalentModifierMask: NSCommandKeyMask | NSControlKeyMask];
	[theMiniMenu addItem: menuItem2];
	
	// Create a menu item with standard message
	menuItem2 = NSMenuItem.alloc;
	[menuItem2 initWithTitle: @"Quit" action: @selector(terminate:) keyEquivalent: @"q"];
	[menuItem2 setKeyEquivalentModifierMask: NSCommandKeyMask];
	[theMiniMenu addItem: menuItem2];
	
	// Adding a menu is done with a dummy item to connect the menu to its parent
	dummyItem = NSMenuItem.alloc;
	[dummyItem initWithTitle: @"" action: nil keyEquivalent: @""];
	[dummyItem setSubmenu: theMiniMenu];

	[mainMenu addItem: dummyItem];
}

void glutInit(int *argcp, char **argv)
{
	pool = [NSAutoreleasePool new];
	myApp = [MGApplication sharedApplication];
	
	[NSApp setActivationPolicy: NSApplicationActivationPolicyRegular]; // Thanks to Marcus Stenbäck
	
	gRunning = 1;
	home();
	gettimeofday(&timeStart, NULL);
	CreateMenu();
	myTimerController = [TimerController alloc];

	int i;
	for (i = 0; i < 256; i++) gKeymap[i] = 0;
}

int gWindowPosX = 10;
int gWindowPosY = 50;
int gWindowWidth = 400;
int gWindowHeight = 400;

void glutInitWindowPosition (int x, int y)
{
	gWindowPosX = x;
	gWindowPosY = y;
}
void glutInitWindowSize (int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;
}
void glutCreateWindow (char *windowTitle)
{
// To do: Should get screen height instead of 1000
	NSRect frame = NSMakeRect(gWindowPosX, NSScreen.mainScreen.frame.size.height - gWindowPosY-gWindowHeight, gWindowWidth, gWindowHeight);
	
	window = [NSWindow alloc];
	[window initWithContentRect:frame
					styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask
					backing:NSBackingStoreBuffered
					defer:false];
	
	[window setTitle: [[NSString alloc] initWithCString:windowTitle
				encoding:NSMacOSRomanStringEncoding]];

	view = [TestView alloc];
	[view initWithFrame: frame];
	[window setAcceptsMouseMovedEvents:YES];

	// OpenGL init!
	MakeContext(view);

// Moved from main loop
//	[window setContentView: view];
	[window setDelegate: (TestView*)view];
	[window makeKeyAndOrderFront: nil];
	[window makeFirstResponder: view]; // Added 130214
}

void glutMainLoop()
{
	[window setContentView: view];
//	[window setDelegate: (TestView*)view];
//	[window makeKeyAndOrderFront: nil];
	// Main loop
//	[myApp run];

	[myApp finishLaunching];
	NSEvent *event;

	while (gRunning)
	{
		[pool release];
		pool = [NSAutoreleasePool new];
		
		if (updatePending || gIdle != NULL) // If it is, then the setNeedsDisplay below has been called and we will get an update event - but must not block!
		// (If there was an update coming I think it should not block, but at least this works.)
		event = [myApp nextEventMatchingMask: NSAnyEventMask
							untilDate: [NSDate dateWithTimeIntervalSinceNow: 0.0]
//							untilDate: [NSDate distantFuture]
							inMode: NSDefaultRunLoopMode
							dequeue: true
							];
		else
		event = [myApp nextEventMatchingMask: NSAnyEventMask
//							untilDate: [NSDate dateWithTimeIntervalSinceNow: 0.0]
							untilDate: [NSDate distantFuture]
							inMode: NSDefaultRunLoopMode
							dequeue: true
							];
		
		[myApp sendEvent: event];
		[myApp updateWindows];
	
		if (gIdle != NULL)
			if (!updatePending)
				gIdle();
		
		// Did not help
		if (updatePending)
			[theView setNeedsDisplay: YES];
	}
}

// This won't work yet
void glutCheckLoop()
{
	[myApp runOnce];
	
	if (gIdle != NULL)
		if (!updatePending)
			gIdle();
	
	[pool release];
	pool = [NSAutoreleasePool new];
}

void glutTimerFunc(int millis, void (*func)(int arg), int arg)
{
	TimerInfoRec *timerInfo = [TimerInfoRec alloc];
	timerInfo->arg = arg;
	timerInfo->func = func;

	gTimer = [NSTimer
		scheduledTimerWithTimeInterval: millis/1000.0
		target: myTimerController
		selector: @selector(timerFireMethod:)
		userInfo: timerInfo
		repeats: NO];
}

// Added by Ingemar
void glutRepeatingTimer(int millis)
{
	gTimer = [NSTimer
		scheduledTimerWithTimeInterval: millis/1000.0
		target: myTimerController
		selector: @selector(timerFireMethod:)
		userInfo: nil
		repeats: YES];
}

// Bad name, will be removed
void glutRepeatingTimerFunc(int millis)
{
	glutRepeatingTimer(millis);
}

void glutDisplayFunc(void (*func)(void))
{
	gDisplay = func;
}

void glutReshapeFunc(void (*func)(int width, int height))
{
	gReshape = func;
}

void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y))
{
	gKey = func;
}

void glutKeyboardUpFunc(void (*func)(unsigned char key, int x, int y))
{
	gKeyUp = func;
}

void glutSpecialFunc(void (*func)(unsigned char key, int x, int y))
{
	gSpecialKey = func;
}

void glutSpecialUpFunc(void (*func)(unsigned char key, int x, int y))
{
	gSpecialKeyUp = func;
}

void glutPassiveMotionFunc(void (*func)(int x, int y))
{
	gMouseMoved = func;
}

void glutMotionFunc(void (*func)(int x, int y))
{
	gMouseDragged = func;
}

void glutMouseFunc(void (*func)(int button, int state, int x, int y))
{
	gMouseFunc = func;
}

// You can safely skip this
void glutSwapBuffers()
{
	glFlush();
//	[m_context flushBuffer]; Seems to cause problems with flicker!
}

int glutGet(int type)
{
	struct timeval tv;
	
	switch (type)
	{
	case GLUT_WINDOW_WIDTH:
		return lastWidth;
		break;
	case GLUT_WINDOW_HEIGHT:
		return lastHeight;
		break;
	case GLUT_ELAPSED_TIME:
		gettimeofday(&tv, NULL);
		return (tv.tv_usec - timeStart.tv_usec) / 1000 + (tv.tv_sec - timeStart.tv_sec)*1000;
		break;
	}
}

void glutInitDisplayMode(unsigned int mode)
{
	gContextInitMode = mode;
}

void glutIdleFunc(void (*func)(void))
{
// glutIdleFunc not recommended.
	gIdle = func;
}

void glutReshapeWindow(int width, int height)
{
	NSRect r;
	
	r = [window frame];
	r.size.width = width;
	r.size.height = height;
	[window setFrame: r display: true];
}

void glutSetWindowTitle(char *title)
{
	[window setTitle: [NSString stringWithUTF8String: title]];
}

char glutKeyIsDown(unsigned char c)
{
	return gKeymap[(unsigned int)c];
}

void glutInitContextVersion(int major, int minor)
{
	gContextVersionMajor = major;
	gContextVersionMinor = minor;
}

// glutInitContextFlags(int flags); Add this?

// Visibility: Just call back immediately and claim we are visible!
void glutVisibilityFunc(void (*visibility)(int status))
{
	visibility(GLUT_VISIBLE);
}

// THIS IS NOT CORRECT YET
void glutWarpPointer(int x, int y)
{
	NSPoint pp, mp;
	CGPoint pt;
	pp.x = x;
	pp.y = y;

// [NSScreen frame]
//	CGRect r = [window frame];
	
	mp.x = 0; mp.y = 0;
	mp = [window convertBaseToScreen: mp];
	printf("%f %f \n", mp.x, mp.y); // Ger orimliga värden

//   pp = [NSView convertPoint: pp toView: theView];

	pt.x = x + mp.x;
	pt.y = y + mp.y;
	CGPostMouseEvent( pt, 1, 1, 0, 0);
}

// Should be used for auto-show-hide on activate/deactivate
// (Not yet implemented)
char hidden = 0;

void glutShowCursor()
{
	if (hidden)
	{
		[NSCursor unhide];
		hidden = 0;
	}
}
void glutHideCursor()
{
	if (!hidden)
	{
		[NSCursor hide];
		hidden = 1;
	}
}

