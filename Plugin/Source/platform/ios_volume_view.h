/**
 * @file   ios_volume_view.h
 * @brief  iOS-only native system volume slider using MPVolumeView.
 */

#ifndef ENAGA_PLATFORM_IOS_VOLUME_VIEW_H_
#define ENAGA_PLATFORM_IOS_VOLUME_VIEW_H_

#if JUCE_IOS

#include <juce_gui_extra/juce_gui_extra.h>

/**
 * Wraps Apple's MPVolumeView inside a JUCE component.
 *
 * MPVolumeView is the system-standard volume control widget on iOS.
 * It controls the device media volume directly and renders with the
 * native iOS appearance (speaker icons, system-style track/thumb).
 * No custom painting is performed; appearance is entirely determined
 * by UIKit.
 */
class IOSVolumeView final : public juce::UIViewComponent {
 public:
  IOSVolumeView();
  ~IOSVolumeView() override;
};

#endif  // JUCE_IOS

#endif  // ENAGA_PLATFORM_IOS_VOLUME_VIEW_H_
