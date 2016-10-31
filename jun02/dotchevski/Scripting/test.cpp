
#ifdef _MSC_VER //Detect Microsoft C++ compiler

#pragma inline_depth(255)
#pragma warning( disable: 4786 ) //identifier was truncated to '255' characters in the debug information
#pragma warning( disable: 4100 ) //unreferenced formal parameter (Warning level 4)
#pragma warning( disable: 4710 ) //function not inlined (Warning level 4)

//Microsoft compiler version 12 (MSVC 6) has some problems with the
//typename keyword. Works fine without typename. 
#if _MSC_VER<1300
#define typename
#endif

#endif //_MSC_VER




#include "scripting.h"

#include <set>




//Define Root
class Root
{
public:
	virtual ~Root() { }
};
SELECTROOT_SUPPORT(Root,10)


//Define the Is<> template. This must be done after Root has been defined.
namespace scripting
{
	template <class T>
	struct Is: public expr_base<Is<T> >
	{
		typedef Root input_type;
		typedef T output_type;
		bool operator()( const input_type* obj ) const { return 0!=dynamic_cast<const T*>(obj); }
	};
}




//Define Actor
class Actor: public Root
{
public:
	bool HasWeapon() const { return false; }
	bool HasArmor() const { return false; }
	float GetHealth() const { return 0; }
	void SetHealth( float health ) { }
	void Attack() { }
	void DoNothing() const { };
};
SELECTROOT_SUPPORT(Actor,20)

//Actor functors
namespace scripting
{
	struct HasWeapon: public expr_base<HasWeapon>
	{
		typedef Actor input_type;
		typedef Actor output_type;
		bool operator()( const input_type* obj ) const { return obj->HasWeapon(); }
	};

	struct HasArmor: public expr_base<HasArmor>
	{
		typedef Actor input_type;
		typedef Actor output_type;
		bool operator()( const input_type* obj ) const { return obj->HasArmor(); }
	};

	struct Health: public expr_base<Health>
	{
		typedef Actor input_type;
		typedef Actor output_type;
		float operator()( const input_type* obj ) const { return obj->GetHealth(); }
	};

	struct SetHealth
	{
		float health_;
		SetHealth( float health ): health_(health) { }
		void operator()( Actor* obj ) const { obj->SetHealth(health_); }
	};

	struct Attack
	{
		void operator()( Actor* obj ) const { obj->Attack(); }
	};

	struct DoNothing
	{
		void operator()( const Actor* obj ) const { obj->DoNothing(); }
	};
}




//Define Grunt.
class Grunt: public Actor
{
};
SELECTROOT_SUPPORT(Grunt,30)



//Test
int main()
{
	using namespace std;
	using namespace scripting;

	//Test the basic predicate support
	set<Grunt*> grunts;
	X( grunts, SetHealth(5) );
	X( grunts, HasWeapon() || HasArmor(), Attack() );
	X( grunts, !HasArmor() && Health()<5, SetHealth(5) );

	//Test the type predicate support
	set<Root*> objects;
	X( objects, Is<Actor>(), Attack() );
	X( objects, Is<Actor>() && Health()>5, Attack() );
	X( objects, Is<Actor>() && HasArmor(), Attack() );
	X( objects, Is<Actor>() && (HasArmor()||HasWeapon()), Attack() );

	//Test the const compliance of the basic predicate support
	set<const Actor*> constActors;
	X( constActors, DoNothing() );
//	X( constActors, SetHealth(5) ); //Should not compile

	//Test the const compliance of the type predicates support
	set<const Root*> constObjects;
	X( constObjects, Is<Actor>(), DoNothing() );
//	X( constObjects, Is<Actor>(), SetHealth(5) ); //Should not compile

	return 0;
}
