/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
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

#ifndef ROMMANAGER_H
#define ROMMANAGER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "rom.h"

namespace definition {
struct Main;
using MainPtr = std::shared_ptr<Main>;
}

/**
 * Manages ROM files and metadata
 */
class RomManager : public QObject {
    Q_OBJECT
public:
    static RomManager *get();

    RomManager(const RomManager &) = delete;
    RomManager &operator=(const RomManager &) = delete;

    /* Loads rom list and metadata. */
    void load();

    /* Saves rom list and metadata */
    void save();

    const std::vector<RomMeta> &roms() { return roms_; }

    void addRom(const std::string &name, const definition::MainPtr &definition,
                const uint8_t *data, size_t size);

    /* Returns the ROM with id or nullptr if the ROM does
     * not exist. Be careful not to store this reference
     * as ROMs can be added or removed. */
    const RomMeta *fromId(int id) const;

    /* Loads a ROM from a ROM id. May throw an exception if the id
     * does not exist or if the ROM could not be loaded */
    std::shared_ptr<Rom> loadId(int id);

private:
    RomManager() = default;
    std::vector<RomMeta> roms_;
    int nextId_{};

    void readRoms(QXmlStreamReader &xml);

signals:
    void updateRoms();
};

#endif // ROMMANAGER_H
