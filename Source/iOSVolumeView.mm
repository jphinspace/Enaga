/**
 * @file   iOSVolumeView.mm
 * @brief  iOS-only native system volume slider implementation.
 *
 * Compiled only on iOS (Xcode treats .mm as Objective-C++).
 * Uses MPVolumeView from MediaPlayer.framework to embed Apple's
 * standard system volume control into the JUCE component hierarchy.
 */

#include "iOSVolumeView.h"

#if JUCE_IOS

#import <MediaPlayer/MediaPlayer.h>

iOSVolumeView::iOSVolumeView()
{
    // Create the native volume slider. [[alloc] init] gives us a +1 retain
    // count. JUCE's UIViewComponent stores it with an ARC strong reference
    // inside setView(), so the view remains alive after this scope exits
    // and the local strong reference is released by ARC.
    auto* volView = [[MPVolumeView alloc] init];
    setView((__bridge void*) volView);
}

iOSVolumeView::~iOSVolumeView()
{
    // Remove the native view from the hierarchy before JUCE tears down
    // the component tree to avoid dangling UIView references.
    setView(nullptr);
}

#endif // JUCE_IOS
