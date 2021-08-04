// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2020-2021 Intel Corporation. All Rights Reserved.

#pragma once

#include "RealSenseID/DeviceConfig.h"
#include "RealSenseID/AuthenticationCallback.h"
#include "RealSenseID/AuthFaceprintsExtractionCallback.h"
#include "RealSenseID/EnrollFaceprintsExtractionCallback.h"
#include "RealSenseID/EnrollmentCallback.h"
#include "RealSenseID/Faceprints.h"
#include "RealSenseID/SerialConfig.h"
#include "RealSenseID/SignatureCallback.h"
#include "RealSenseID/Status.h"
#include "RealSenseID/MatcherDefines.h"

#ifdef ANDROID
#include "RealSenseID/AndroidSerialConfig.h"
#endif
#ifdef RSID_SECURE
#include "PacketManager/SecureSession.h"
using Session = RealSenseID::PacketManager::SecureSession;
#else
#include "PacketManager/NonSecureSession.h"
using Session = RealSenseID::PacketManager::NonSecureSession;
#endif // RSID_SECURE

#include <memory>
#include <atomic>
#include <chrono>

namespace RealSenseID
{
class FaceAuthenticatorImpl
{
public:
    explicit FaceAuthenticatorImpl(SignatureCallback* callback);

    ~FaceAuthenticatorImpl() = default;

    FaceAuthenticatorImpl(const FaceAuthenticatorImpl&) = delete;
    FaceAuthenticatorImpl& operator=(const FaceAuthenticatorImpl&) = delete;

    Status Connect(const SerialConfig& config);
#ifdef ANDROID
    Status Connect(const AndroidSerialConfig& config);
#endif

    void Disconnect();

#ifdef RSID_SECURE
    Status Pair(const char* ecdsaHostPubKey, const char* ecdsaHostPubKeySig, char* ecdsaDevicePubKey);
    Status Unpair();    
#endif

    Status Enroll(EnrollmentCallback& callback, const char* user_id);
    EnrollStatus EnrollImage(const char* user_id, const unsigned char* buffer, unsigned int width, unsigned int height);
    Status Authenticate(AuthenticationCallback& callback);
    Status AuthenticateLoop(AuthenticationCallback& callback);
    Status Cancel();
    Status RemoveUser(const char* user_id);
    Status RemoveAll();

    Status SetDeviceConfig(const DeviceConfig& device_config);
    Status QueryDeviceConfig(DeviceConfig& device_config);
    Status QueryUserIds(char** user_ids, unsigned int& number_of_users);
    Status QueryNumberOfUsers(unsigned int& number_of_users);
    Status Standby();

    Status ExtractFaceprintsForEnroll(EnrollFaceprintsExtractionCallback& callback);
    Status ExtractFaceprintsForAuth(AuthFaceprintsExtractionCallback& callback);
    Status ExtractFaceprintsForAuthLoop(AuthFaceprintsExtractionCallback& callback);
    
    MatchResultHost MatchFaceprints(MatchElement& new_faceprints, Faceprints& existing_faceprints,
                                    Faceprints& updated_faceprints, ThresholdsConfidenceEnum matcher_confidence_level=ThresholdsConfidenceEnum::ThresholdsConfidenceLevel_High);

    Status GetUsersFaceprints(Faceprints* user_features, unsigned int& num_of_users);
    Status SetUsersFaceprints(UserFaceprints* users_faceprints, unsigned int num_of_users);

private:
#ifndef STM32_HAL
#ifdef RSID_SECURE
    // in secure mode sleep less to take into account start session duration
    const std::chrono::milliseconds _loop_interval_no_face {1500};
    const std::chrono::milliseconds _loop_interval_with_face {100};
#else
    const std::chrono::milliseconds _loop_interval_no_face {2100};
    const std::chrono::milliseconds _loop_interval_with_face {600};
#endif
#else
#ifdef RSID_SECURE
    // in secure mode sleep less to take into account start session duration
    const uint32_t _loop_interval_no_face {1500};
    const uint32_t _loop_interval_with_face {100};
#else
    const uint32_t _loop_interval_no_face {2100};
    const uint32_t _loop_interval_with_face {600};
#endif
#endif
    std::atomic<bool> _cancel_loop {false};
    std::unique_ptr<PacketManager::SerialConnection> _serial;
    Session _session;

    // wait for cancel flag while sleeping upto timeout
#ifndef STM32_HAL
    void AuthLoopSleep(std::chrono::milliseconds timeout);
#else
    void AuthLoopSleep(uint32_t time_out); // using timeout_t = uint32_t;
#endif
    static bool ValidateUserId(const char* user_id);
};
} // namespace RealSenseID
