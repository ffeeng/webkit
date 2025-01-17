/*
 * Copyright (C) 2016-2021 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "DisplayList.h"
#include <memory>

namespace WebCore {
namespace DisplayList {

class InMemoryDisplayList : public DisplayList {
public:
    WEBCORE_EXPORT InMemoryDisplayList();

    WEBCORE_EXPORT ~InMemoryDisplayList();

    class WEBCORE_EXPORT WritingClient : public ItemBufferWritingClient {
        WTF_MAKE_FAST_ALLOCATED;
    private:
        Optional<std::size_t> requiredSizeForItem(ItemHandle) const final;
        void encodeItemInline(ItemHandle, uint8_t*) const final;
    };

    class WEBCORE_EXPORT ReadingClient : public ItemBufferReadingClient {
        WTF_MAKE_FAST_ALLOCATED;
    private:
        Optional<ItemHandle> WARN_UNUSED_RETURN decodeItem(const uint8_t*, size_t, ItemType, uint8_t*) final;
    };

private:
    std::unique_ptr<WritingClient> m_writingClient;
    std::unique_ptr<ReadingClient> m_readingClient;
};

}
}
