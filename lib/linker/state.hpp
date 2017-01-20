#ifndef STATE_H
#define STATE_H
#include "helper/debug.hpp"
#include "helper/help.hpp"
#include "geom3d/geom3d.hpp"
#include "pdbreader/bond.hpp"
#include "pdbreader/atom.hpp"
#include <tuple>
#include <functional>

using namespace std;

namespace Molib {
	class Atom;
};

namespace Linker {
	class Segment;
	class State {
	public:
		struct comp { bool operator()(State* const i, State* const j) const
			{ return i->get_id() < j->get_id(); } };

		typedef vector<State*> Vec;
		typedef vector<State> NVec;
		typedef set<State*, State::comp> Set;
		typedef vector<const State*> ConstVec;
		typedef pair<const State*, const State*> ConstPair;
		typedef int Id;
		
	private:
		const Segment &__segment;
		Geom3D::Point::Vec __crds;
		double __energy;
		Id __id;
#ifndef NDEBUG
		int __no;
#endif		
	public:
		State(const Segment &segment, const Geom3D::Point::Vec crds, const double energy=0) : 
			__segment(segment), __crds(crds), __energy(energy) {}
		void set_energy(const double energy) { __energy = energy; }
		double get_energy() const { return __energy; }			
		const Segment& get_segment() const { return __segment; }			
		const Geom3D::Point::Vec& get_crds() const { return __crds; }			
		Geom3D::Point::Vec& get_crds() { return __crds; }			
		const Geom3D::Point& get_crd(const int i) const { return __crds[i]; }
		bool clashes(const State &other, const double clash_coeff) const; // clashes between this and other state
		string pdb() const;
		void set_id(Id id) { __id = id; }
                Id get_id() const { return __id; }
#ifndef NDEBUG
		void set_no(int no) { __no = no; }
		int get_no() const { return __no; }
#endif
		friend ostream& operator<< (ostream& stream, const State& s);
		friend ostream& operator<< (ostream& stream, const Vec& sv);
	};
	
	State::Vec operator-(const State::Set& left, const State::Set& right);
};
#endif
