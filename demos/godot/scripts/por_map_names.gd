extends RefCounted
## Reference labels, not GEO text or runtime-verified area identities.
## Gold Box Explorer eac30abaa6ee66aea6f5d65ebe6d676b10015a8f,
## src/Common/Plugins/GeoDax/GeoDaxFile.cs, PopulateMapNames (Pool of Radiance).
## Archive assignments come from the inspected DOS installation. Preserve the
## full source key so unfamiliar banks do not inherit a same-record-ID guess.
const SOURCE := "Gold Box Explorer reference names; not runtime-verified."
const NAMES := {
	"GEO1.DAX:18": "Podal Plaza",
	"GEO1.DAX:24": "Temple of Bane",
	"GEO1.DAX:31": "Wealthy Area",
	"GEO2.DAX:9": "Stojanow Gate",
	"GEO2.DAX:15": "Mendor's Library",
	"GEO2.DAX:20": "Slums",
	"GEO3.DAX:0": "Civilized Area, New Phlan",
	"GEO3.DAX:14": "Kovel Mansion",
	"GEO4.DAX:2": "Cadorna Textile House",
	"GEO4.DAX:10": "Valhingen Graveyard",
	"GEO4.DAX:21": "Sokal Keep",
	"GEO5.DAX:3": "Valjevo Castle, North West",
	"GEO5.DAX:4": "Valjevo Castle, North East",
	"GEO5.DAX:5": "Valjevo Castle, South East",
	"GEO5.DAX:6": "Valjevo Castle, South West",
	"GEO5.DAX:7": "Valjevo Castle, Inner Tower",
	"GEO6.DAX:1": "Buccaneer Base",
	"GEO6.DAX:25": "Unknown Lair",
	"GEO6.DAX:28": "Outpost of Zhentil Keep",
	"GEO7.DAX:17": "Nomad Camp",
	"GEO7.DAX:22": "Sorcerer's Island, Level 1",
	"GEO7.DAX:23": "Sorcerer's Island, Level 2 and 3",
	"GEO7.DAX:26": "Unknown Zone",
	"GEO8.DAX:13": "Kobold Caves",
	"GEO8.DAX:16": "Lizard Men Keep",
	"GEO8.DAX:27": "Unknown Lair",
	"GEO8.DAX:29": "Kuto's Well",
	"GEO8.DAX:30": "Lizard Men Catacombs",
	"GEO8.DAX:32": "Kuto's Well Catacombs",
}

static func location(archive: String, record: int) -> String:
	return NAMES.get("%s:%d" % [archive.to_upper(), record], "Unknown area")
