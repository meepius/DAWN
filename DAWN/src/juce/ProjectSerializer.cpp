#include "juce/ProjectSerializer.h"

#include "juce/Vst3Wrapper.h"
#include "model/Paths.h"

namespace dawn {
namespace {

void relativizeAudio(juce::ValueTree tree, const juce::String& root) {
    if (tree.hasType(Id::clip)) {
        const auto stored = tree[Id::audio].toString();
        if (stored.isNotEmpty()) {
            tree.setProperty(Id::audio, juce::String(toRelativePath(root.toStdString(), stored.toStdString())), nullptr);
        }
    }
    for (auto child : tree) relativizeAudio(child, root);
}

void stampPluginState(juce::ValueTree tree, PluginRack& rack) {
    if (tree.hasType(Id::device)) {
        const auto id = static_cast<std::uint64_t>(static_cast<juce::int64>(tree[Id::deviceId]));
        if (auto* device = rack.find(id)) {
            juce::MemoryBlock block;
            if (device->getStateInformation(block)) {
                tree.setProperty(Id::pluginState, block.toBase64Encoding(), nullptr);
            }
        }
    }
    for (auto child : tree) stampPluginState(child, rack);
}

void restorePlugins(const juce::ValueTree& tree, PluginRack& rack, juce::AudioPluginFormatManager& formats, juce::String& error) {
    if (tree.hasType(Id::device)) {
        const auto id = static_cast<std::uint64_t>(static_cast<juce::int64>(tree[Id::deviceId]));
        const auto identifier = tree[Id::identifier].toString();
        if (id != 0 && identifier.isNotEmpty()) {
            juce::PluginDescription description;
            description.pluginFormatName = tree[Id::formatName].toString().isEmpty() ? "VST3" : tree[Id::formatName].toString();
            description.fileOrIdentifier = identifier;
            description.name = tree[Id::name].toString();
            juce::String createError;
            auto wrapper = Vst3Wrapper::create(formats, description, 48000.0, 512, createError);
            if (wrapper == nullptr) {
                if (error.isNotEmpty()) error += "\n";
                error += createError.isEmpty() ? "Could not open " + identifier : createError;
            } else {
                juce::MemoryBlock block;
                block.fromBase64Encoding(tree[Id::pluginState].toString());
                if (block.getSize() > 0) wrapper->setStateInformation(block.getData(), static_cast<int>(block.getSize()));
                rack.add(id, std::move(wrapper));
            }
        }
    }
    for (const auto& child : tree) restorePlugins(child, rack, formats, error);
}

} // namespace

bool ProjectSerializer::save(const ProjectState& state, PluginRack& rack, const juce::File& file) const {
    auto copy = state.state().createCopy();
    const auto root = file.getParentDirectory().getFullPathName();
    relativizeAudio(copy, root);
    stampPluginState(copy, rack);
    const auto xml = copy.createXml();
    if (xml == nullptr) return false;
    file.getParentDirectory().createDirectory();
    return xml->writeTo(file);
}

bool ProjectSerializer::load(ProjectState& state,
                             PluginRack& rack,
                             juce::AudioPluginFormatManager& formats,
                             const juce::File& file,
                             juce::String& error) const {
    const auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr) {
        error = "The project file could not be read.";
        return false;
    }
    auto tree = juce::ValueTree::fromXml(*xml);
    if (!tree.hasType(Id::project)) {
        error = "This is not a DAWN project.";
        return false;
    }
    rack.clear();
    state.replace(std::move(tree));
    restorePlugins(state.state(), rack, formats, error);
    return true;
}

namespace {

juce::String trackIdString(const juce::ValueTree& track) { return "t" + track[Id::trackId].toString(); }

void addNotes(juce::XmlElement& parent, const juce::ValueTree& clip) {
    auto* notes = parent.createNewChildElement("Notes");
    for (const auto& note : clip) {
        if (!note.hasType(Id::note)) continue;
        auto* element = notes->createNewChildElement("Note");
        element->setAttribute("time", static_cast<double>(note[Id::start]));
        element->setAttribute("duration", static_cast<double>(note[Id::length]));
        element->setAttribute("channel", 0);
        element->setAttribute("key", static_cast<int>(note[Id::pitch]));
        element->setAttribute("vel", static_cast<double>(note[Id::velocity]) / 127.0);
    }
}

juce::ValueTree treeFromDawproject(const juce::XmlElement& project, const juce::File& mediaRoot) {
    juce::ValueTree tree{Id::project};
    if (auto* transport = project.getChildByName("Transport")) {
        if (auto* tempo = transport->getChildByName("Tempo")) tree.setProperty(Id::bpm, tempo->getDoubleAttribute("value", 120.0), nullptr);
    }
    std::uint64_t next = 1;
    if (auto* structure = project.getChildByName("Structure")) {
        for (auto* trackXml = structure->getFirstChildElement(); trackXml != nullptr; trackXml = trackXml->getNextElement()) {
            if (!trackXml->hasTagName("Track")) continue;
            auto track = juce::ValueTree{Id::track};
            const auto id = next++;
            track.setProperty(Id::trackId, static_cast<juce::int64>(id), nullptr);
            track.setProperty(Id::name, trackXml->getStringAttribute("name"), nullptr);
            track.setProperty(Id::userNotes, trackXml->getStringAttribute("comment"), nullptr);
            const auto content = trackXml->getStringAttribute("contentType");
            track.setProperty(Id::kind, static_cast<int>(content == "audio" ? TrackKind::Audio : TrackKind::Midi), nullptr);
            track.setProperty(Id::volume, 0.8, nullptr);
            if (auto* channel = trackXml->getChildByName("Channel")) {
                if (auto* devices = channel->getChildByName("Devices")) {
                    for (auto* deviceXml = devices->getFirstChildElement(); deviceXml != nullptr; deviceXml = deviceXml->getNextElement()) {
                        auto device = juce::ValueTree{Id::device};
                        device.setProperty(Id::deviceId, static_cast<juce::int64>(next++), nullptr);
                        device.setProperty(Id::name, deviceXml->getStringAttribute("deviceName"), nullptr);
                        device.setProperty(Id::identifier, deviceXml->getStringAttribute("deviceID"), nullptr);
                        device.setProperty(Id::formatName, deviceXml->hasTagName("ClapPlugin") ? "CLAP" : "VST3", nullptr);
                        device.setProperty(Id::userNotes, deviceXml->getStringAttribute("comment"), nullptr);
                        if (auto* state = deviceXml->getChildByName("State")) device.setProperty(Id::pluginState, state->getAllSubText(), nullptr);
                        track.appendChild(device, nullptr);
                    }
                }
            }
            tree.appendChild(track, nullptr);
        }
    }
    if (auto* arrangement = project.getChildByName("Arrangement")) {
        if (auto* lanes = arrangement->getChildByName("Lanes")) {
            for (auto* lane = lanes->getFirstChildElement(); lane != nullptr; lane = lane->getNextElement()) {
                if (!lane->hasTagName("Lanes")) continue;
                const auto trackKey = lane->getStringAttribute("track");
                juce::ValueTree track;
                for (auto child : tree) {
                    if (child.hasType(Id::track) && trackIdString(child) == trackKey) track = child;
                }
                if (!track.isValid() || lane->getChildByName("Clips") == nullptr) continue;
                for (auto* clipXml = lane->getChildByName("Clips")->getFirstChildElement(); clipXml != nullptr; clipXml = clipXml->getNextElement()) {
                    if (!clipXml->hasTagName("Clip")) continue;
                    auto clip = juce::ValueTree{Id::clip};
                    clip.setProperty(Id::loopBeats, clipXml->getDoubleAttribute("duration", 4.0), nullptr);
                    clip.setProperty(Id::userNotes, clipXml->getStringAttribute("comment"), nullptr);
                    if (auto* audio = clipXml->getChildByName("Audio")) {
                        if (auto* file = audio->getChildByName("File")) {
                            clip.setProperty(Id::audio, mediaRoot.getChildFile(file->getStringAttribute("path")).getFullPathName(), nullptr);
                        }
                    }
                    if (auto* notes = clipXml->getChildByName("Notes")) {
                        for (auto* noteXml = notes->getFirstChildElement(); noteXml != nullptr; noteXml = noteXml->getNextElement()) {
                            if (!noteXml->hasTagName("Note")) continue;
                            auto note = juce::ValueTree{Id::note};
                            note.setProperty(Id::start, noteXml->getDoubleAttribute("time"), nullptr);
                            note.setProperty(Id::length, noteXml->getDoubleAttribute("duration", 0.25), nullptr);
                            note.setProperty(Id::pitch, noteXml->getIntAttribute("key", 60), nullptr);
                            note.setProperty(Id::velocity, juce::roundToInt(noteXml->getDoubleAttribute("vel", 0.8) * 127.0), nullptr);
                            clip.appendChild(note, nullptr);
                        }
                    }
                    track.appendChild(clip, nullptr);
                }
            }
        }
    }
    tree.setProperty(Id::nextId, static_cast<juce::int64>(next), nullptr);
    return tree;
}

} // namespace

bool ProjectSerializer::exportDawproject(const ProjectState& state, PluginRack& rack, const juce::File& file) const {
    juce::XmlElement project{"Project"};
    project.setAttribute("version", "1.0");
    auto* application = project.createNewChildElement("Application");
    application->setAttribute("name", "DAWN");
    application->setAttribute("version", "0.1.0");
    auto* tempo = project.createNewChildElement("Transport")->createNewChildElement("Tempo");
    tempo->setAttribute("value", static_cast<double>(state.state()[Id::bpm]));
    tempo->setAttribute("unit", "bpm");
    auto* structure = project.createNewChildElement("Structure");
    auto* arrangement = project.createNewChildElement("Arrangement")->createNewChildElement("Lanes");
    arrangement->setAttribute("timeUnit", "beats");
    juce::ZipFile::Builder zip;
    for (const auto& trackTree : state.state()) {
        if (!trackTree.hasType(Id::track)) continue;
        auto* track = structure->createNewChildElement("Track");
        const auto id = trackIdString(trackTree);
        track->setAttribute("id", id);
        track->setAttribute("name", trackTree[Id::name].toString());
        track->setAttribute("loaded", "true");
        track->setAttribute("comment", trackTree[Id::userNotes].toString());
        const auto kind = static_cast<TrackKind>(static_cast<int>(trackTree[Id::kind]));
        track->setAttribute("contentType", kind == TrackKind::Audio ? "audio" : "notes");
        auto* devices = track->createNewChildElement("Channel")->createNewChildElement("Devices");
        devices->getParentElement()->setAttribute("audioChannels", 2);
        devices->getParentElement()->setAttribute("role", "regular");
        for (const auto& deviceTree : trackTree) {
            if (!deviceTree.hasType(Id::device)) continue;
            const auto format = deviceTree[Id::formatName].toString();
            auto* device = devices->createNewChildElement(format.equalsIgnoreCase("CLAP") ? "ClapPlugin" : "Vst3Plugin");
            device->setAttribute("deviceID", deviceTree[Id::identifier].toString());
            device->setAttribute("deviceName", deviceTree[Id::name].toString());
            device->setAttribute("loaded", "true");
            device->setAttribute("comment", deviceTree[Id::userNotes].toString());
            juce::String encoded = deviceTree[Id::pluginState].toString();
            if (auto* live = rack.find(static_cast<std::uint64_t>(static_cast<juce::int64>(deviceTree[Id::deviceId])))) {
                juce::MemoryBlock block;
                if (live->getStateInformation(block)) encoded = block.toBase64Encoding();
            }
            if (encoded.isNotEmpty()) device->createNewChildElement("State")->addTextElement(encoded);
        }
        auto* lane = arrangement->createNewChildElement("Lanes");
        lane->setAttribute("track", id);
        auto* clips = lane->createNewChildElement("Clips");
        for (const auto& clipTree : trackTree) {
            if (!clipTree.hasType(Id::clip)) continue;
            auto* clip = clips->createNewChildElement("Clip");
            clip->setAttribute("time", static_cast<double>(clipTree[Id::launchBeat]));
            clip->setAttribute("duration", static_cast<double>(clipTree[Id::loopBeats]));
            clip->setAttribute("comment", clipTree[Id::userNotes].toString());
            const auto audio = clipTree[Id::audio].toString();
            if (audio.isNotEmpty()) {
                const juce::File source(audio);
                const auto stored = "audio/" + source.getFileName();
                auto* fileElement = clip->createNewChildElement("Audio")->createNewChildElement("File");
                fileElement->setAttribute("path", stored);
                fileElement->setAttribute("external", "false");
                if (source.existsAsFile()) zip.addEntry(new juce::FileInputStream(source), 9, stored, source.getLastModificationTime());
            } else {
                addNotes(*clip, clipTree);
            }
        }
    }
    const auto xml = project.toString();
    juce::MemoryBlock xmlBlock(xml.toRawUTF8(), xml.getNumBytesAsUTF8());
    zip.addEntry(new juce::MemoryInputStream(xmlBlock, false), 9, "project.xml", juce::Time::getCurrentTime());
    file.deleteFile();
    auto stream = file.createOutputStream();
    if (stream == nullptr) return false;
    return zip.writeToStream(*stream, nullptr);
}

bool ProjectSerializer::importDawproject(ProjectState& state,
                                         PluginRack& rack,
                                         juce::AudioPluginFormatManager& formats,
                                         const juce::File& file,
                                         juce::String& error) const {
    juce::ZipFile zip(file);
    if (zip.getNumEntries() <= 0) {
        error = "The dawproject archive could not be read.";
        return false;
    }
    const auto media = file.getParentDirectory().getChildFile(file.getFileNameWithoutExtension() + "-media");
    media.createDirectory();
    if (!zip.uncompressTo(media)) {
        error = "The dawproject archive could not be extracted.";
        return false;
    }
    const auto xml = juce::XmlDocument::parse(media.getChildFile("project.xml"));
    if (xml == nullptr || !xml->hasTagName("Project")) {
        error = "project.xml is missing from the dawproject.";
        return false;
    }
    auto tree = treeFromDawproject(*xml, media);
    rack.clear();
    state.replace(std::move(tree));
    restorePlugins(state.state(), rack, formats, error);
    return true;
}

} // namespace dawn
