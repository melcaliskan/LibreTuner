/*
 * LibreTuner
 * Copyright (C) 2018  Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "udsauthenticator.h"
#include "logger.h"

#include <cassert>
#include <sstream>

namespace uds {

void Authenticator::auth(const std::string &key, uds::Protocol &uds,
                         uint8_t sessionType) {
    key_ = key;
    uds_ = &uds;

    do_session(sessionType);
}



void Authenticator::do_session(uint8_t sessionType) {
    Logger::debug("[AUTH] sending session request");
    uds_->requestSession(sessionType);
    do_request_seed();
}



void Authenticator::do_request_seed() {
    Logger::debug("[AUTH] Sending seed request");
    std::vector<uint8_t> seed = uds_->requestSecuritySeed();

    // Generate key from seed
    uint32_t key = generateKey(0xC541A9, seed.data(), seed.size());
    do_send_key(key);
}



void Authenticator::do_send_key(uint32_t key) {
    Logger::debug("[AUTH] Sending key request");
    uint8_t kData[3];
    kData[0] = key & 0xFF;
    kData[1] = (key & 0xFF00) >> 8;
    kData[2] = (key & 0xFF0000) >> 16;

    uds_->requestSecurityKey(kData, 3);
}



uint32_t Authenticator::generateKey(uint32_t parameter,
                                    const uint8_t *seed, size_t size) {
    std::vector<uint8_t> nseed(seed, seed + size);
    nseed.insert(nseed.end(), key_.begin(), key_.end());

    // This is Mazda's key generation algorithm reverse engineered from a
    // Mazda 6 MPS ROM. Internally, the ECU uses a timer/counter for the seed
    // generation

    for (uint8_t c : nseed) {
        for (int r = 8; r > 0; --r) {
            uint8_t s = (c & 1) ^ (parameter & 1);
            uint32_t m = 0;
            if (s != 0) {
                parameter |= 0x1000000;
                m = 0x109028;
            }

            c >>= 1;
            parameter >>= 1;
            uint32_t p3 = parameter & 0xFFEF6FD7;
            parameter ^= m;
            parameter &= 0x109028;

            parameter |= p3;

            parameter &= 0xFFFFFF;
        }
    }

    uint32_t res = (parameter >> 4) & 0xFF;
    res |= (((parameter >> 20) & 0xFF) + ((parameter >> 8) & 0xF0)) << 8;
    res |= (((parameter << 4) & 0xFF) + ((parameter >> 16) & 0x0F)) << 16;

    return res;
}
} // namespace uds
