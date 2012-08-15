#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char color_type;
typedef unsigned char tile_index_type;
struct tile_type {
	typedef std::pair<tile_index_type, color_type> pair_representation;
	typedef short internal_representation;

	internal_representation val;

	tile_type(tile_index_type tile_index, color_type color)
		: val(tile_index | (color << 8))
	{
	}

	tile_type() {}

	pair_representation & pair() {
		return reinterpret_cast<pair_representation &>(val);
	}

	const pair_representation & pair() const {
		return reinterpret_cast<const pair_representation &>(val);
	}

	tile_index_type & tile_index() {
		return pair().first;
	}

	const tile_index_type & tile_index() const {
		return pair().first;
	}

	color_type & color() {
		return pair().second;
	}

	const color_type & color() const {
		return pair().second;
	}

	bool operator==(const tile_type & other) const { return val == other.val; }
	bool operator!=(const tile_type & other) const { return val != other.val; }
};

#endif // __TYPES_H__
// vim:set ts=4 sts=4 sw=4 noet:
