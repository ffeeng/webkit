/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "RemoteImageDecoderAVFManager.h"

#if ENABLE(GPU_PROCESS) && HAVE(AVASSETREADER)

#include "GPUProcessConnection.h"
#include "RemoteImageDecoderAVF.h"
#include "RemoteImageDecoderAVFManagerMessages.h"
#include "RemoteImageDecoderAVFProxyMessages.h"
#include "SharedBufferDataReference.h"
#include "WebProcess.h"

namespace WebKit {

using namespace WebCore;

RefPtr<RemoteImageDecoderAVF> RemoteImageDecoderAVFManager::createImageDecoder(SharedBuffer& data, const String& mimeType, AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption)
{
    if (!m_messageReceiverInitialized) {
        m_messageReceiverInitialized = true;
        gpuProcessConnection().messageReceiverMap().addMessageReceiver(Messages::RemoteImageDecoderAVFManager::messageReceiverName(), *this);
    }

    Optional<ImageDecoderIdentifier> imageDecoderIdentifier;
    IPC::SharedBufferDataReference dataReference { data };
    if (!gpuProcessConnection().connection().sendSync(Messages::RemoteImageDecoderAVFProxy::CreateDecoder(dataReference, mimeType), Messages::RemoteImageDecoderAVFProxy::CreateDecoder::Reply(imageDecoderIdentifier), 0))
        return nullptr;

    if (!imageDecoderIdentifier)
        return nullptr;

    auto remoteImageDecoder = RemoteImageDecoderAVF::create(*this, *imageDecoderIdentifier, data, mimeType);
    m_remoteImageDecoders.add(*imageDecoderIdentifier, makeWeakPtr(remoteImageDecoder.get()));

    return remoteImageDecoder;
}

void RemoteImageDecoderAVFManager::deleteRemoteImageDecoder(const ImageDecoderIdentifier& identifier)
{
    m_remoteImageDecoders.take(identifier);
    gpuProcessConnection().connection().send(Messages::RemoteImageDecoderAVFProxy::DeleteDecoder(identifier), 0);
}

RemoteImageDecoderAVFManager::RemoteImageDecoderAVFManager(WebProcess& process)
    : m_process(process)
{
}

RemoteImageDecoderAVFManager::~RemoteImageDecoderAVFManager()
{
    gpuProcessConnection().messageReceiverMap().removeMessageReceiver(Messages::RemoteImageDecoderAVFManager::messageReceiverName());
}

const char*  RemoteImageDecoderAVFManager::supplementName()
{
    return "RemoteImageDecoderAVFManager";
}

GPUProcessConnection& RemoteImageDecoderAVFManager::gpuProcessConnection() const
{
    return m_process.ensureGPUProcessConnection();
}

void RemoteImageDecoderAVFManager::setUseGPUProcess(bool useGPUProcess)
{
    if (!useGPUProcess) {
        ImageDecoder::resetFactories();
        return;
    }

    ImageDecoder::clearFactories();
    ImageDecoder::installFactory({
        RemoteImageDecoderAVF::supportsMediaType,
        RemoteImageDecoderAVF::canDecodeType,
        [this](SharedBuffer& data, const String& mimeType, AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) {
            return createImageDecoder(data, mimeType, alphaOption, gammaAndColorProfileOption);
        }
    });
}

void RemoteImageDecoderAVFManager::encodedDataStatusChanged(const ImageDecoderIdentifier& identifier, size_t frameCount, const WebCore::IntSize& size, bool hasTrack)
{
    if (!m_remoteImageDecoders.contains(identifier))
        return;

    auto remoteImageDecoder = m_remoteImageDecoders.get(identifier);
    if (!remoteImageDecoder)
        return;

    remoteImageDecoder->encodedDataStatusChanged(frameCount, size, hasTrack);
}

}

#endif
