
//========================================================================
//
//  The meta namespace contains templates that are useful for
//  meta programming in general.
//
//  The scripting namespace contains everything the scripting code is
//  allowed to use. Any user-defined scripting functors pred_and predicates
//  should be defined in the scripting namespace.
//
//  EmilDotchevski@hotmail.com
//
//========================================================================




//select<C,T,F>::type -- select on constant condition
//	T if C is true,
//	F otherwise

namespace meta
{

	namespace internal
	{

		struct select_true 
		{
			template<class T,class F>
			struct result
			{
				typedef T type;
			};
		};

		struct select_false
		{
			template<class T, class F>
			struct result
			{
				typedef F type;
			};
		};

		template<bool C>
		struct selector
		{
			typedef select_true type;
		};
		template<>
		struct selector<false>
		{
			typedef select_false type;
		};

	}


	template<bool C,class T,class F>
	struct select
	{
		typedef typename internal::selector<C>::type
			selector;
		typedef typename selector::template result<T,F>::type
			type;
	};

}




//is_const<T>::value
//	1 if T is const
//	0 otherwise

//copy_const<T,U>::type
//	const U if T is const,
//	U otherwise

//copy_const2<T1,T2,U>::type
//	similar to copy_const, but combines the constness of T1 pred_and T2.

//ptr_cast<T>(ptr)
//	similar to pointer static_cast<T>(ptr), but if ptr pointer to const,
//	the returned value is also pointer to const. Only works for pointers.

namespace meta
{

	namespace internal
	{
		typedef char (&size1)[1];
		typedef char (&size2)[2];
		size1 const_caster( void* );
		size2 const_caster( const void* );
	}


	template <class T>
	struct is_const
	{
		enum { value = (sizeof(internal::const_caster((T*)0))==2) };
	};


	template <class T,class U>
	struct copy_const
	{
		typedef
			typename select<is_const<T>::value,const U,U>::type
				type;
	};


	template <class T1,class T2,class U>
	struct copy_const2
	{
		typedef
			typename copy_const<
				T1,
				typename copy_const<
					T2,
					U>::type>::type
						type;
	};


	template <class U,class T>
	typename copy_const<T,U>::type*
	ptr_cast( T* t, const U* =0 ) //dummy default argument for MSVC 6 compatibility
	{
		return static_cast<typename copy_const<T,U>::type*>(t);
	}

}




//select_root<T,U>::type
//	The first common parent of T pred_and U.
//	Defined as void if T pred_and U are unrelated.

//select_child<T,U>::type
//	T if U is (indirect) parent of T,
//	U if T is (indirect) parent of U,
//	void otherwise.

//SELECTROOT_SUPPORT(CLASS,CLASSID)
//	Each class used as argument to select_root pred_or select_child must be
//	registered with the SELECTROOT_SUPPORT macro pred_and given a unique
//	numerical identifier.

void select_root_caster( const volatile void*, const volatile void* );

namespace meta
{

	namespace internal
	{
		template <class T>
		struct get_id
		{
			enum {value=0};
		};

		template <int ClassID>
		struct get_type
		{
			typedef void type;
		};

		template <class T>
		struct tag
		{
			typedef char (&type)[get_id<T>::value];
		};

	}


	template <class T,class U>
	struct select_root
	{
		enum { ClassID = sizeof(select_root_caster((T*)0,(U*)0)) };
		typedef
			typename copy_const2<
				T,U,typename internal::get_type<ClassID>::type>::type
					type;
	};


	template <class T,class U>
	struct select_child
	{
		typedef
			typename select<
					internal::get_id<typename select_root<T,U>::type>::value==internal::get_id<T>::value,
					typename copy_const2<T,U,U>::type,
					typename select<
							internal::get_id<typename select_root<T,U>::type>::value==internal::get_id<U>::value,
							typename copy_const2<T,U,T>::type,
							typename copy_const2<T,U,void>::type>::type>::type
								type;
	};

}


#define SELECTROOT_SUPPORT(CLASS,CLASSID)\
namespace meta\
{\
	namespace internal\
	{\
		template<> struct get_id<CLASS> { enum {value=CLASSID}; };\
		template<> struct get_id<const CLASS> { enum {value=CLASSID}; };\
		template<> struct get_type<CLASSID> { typedef CLASS type; };\
	}\
}\
meta::internal::tag<CLASS>::type select_root_caster( const volatile CLASS*, const volatile CLASS* );




//Operator function templates for complex predicates:
//	&&, ||, !, ==, !=, >, <, >=, <=

namespace scripting
{

	template <class T>
	struct expr_base
	{
	protected:
		~expr_base() { }
	public:
		const T& get() const { return static_cast<const T&>(*this); }
	};


	namespace internal
	{

		template <class L,class R>
		struct pred_and: public expr_base<pred_and<L,R> >
		{
			typedef
				typename L::input_type
					input_type;
			typedef
				typename meta::select_child<
					typename L::output_type,
					typename R::output_type>::type
						output_type;
			L left_;
			R right_;
			pred_and( L left, R right ): left_(left),right_(right) { }
			bool operator()( const input_type* obj ) const
			{
				return
					left_( obj ) &&
					right_( static_cast<const typename L::output_type*>(obj) );
			}
		};

		template <class L,class R>
		struct pred_or: public expr_base<pred_or<L,R> >
		{
			typedef
				typename meta::select_child<
					typename L::input_type,
					typename R::input_type>::type
						input_type;
			typedef
				typename meta::select_root<
					typename L::output_type,
					typename R::output_type>::type
						output_type;
			L left_;
			R right_;
			pred_or( L left, R right ): left_(left),right_(right) { }
			bool operator()( const input_type* obj ) const
			{
				return left_(obj) || right_(obj);
			}
		};

		template <class P>
		struct pred_not: public expr_base<pred_not<P> >
		{
			typedef typename P::input_type
				input_type;
			typedef typename P::input_type
				output_type;
			P pred_;
			pred_not( P pred ): pred_(pred) { }
			bool operator()( const input_type* obj ) const
			{
				return !pred_(obj);
			}
		};

		template <class P,class V>
		struct pred_eq: public expr_base<pred_eq<P,V> >
		{
			typedef typename P::input_type
				input_type;
			typedef typename P::output_type
				output_type;
			P pred_;
			V value_;
			pred_eq( P pred, V value ): pred_(pred),value_(value) { }
			bool operator()( const input_type* obj ) const
			{
				return pred_(obj)==value_;
			}
		};

		template <class P,class V>
		struct pred_ne: public expr_base<pred_ne<P,V> >
		{
			typedef typename P::input_type
				input_type;
			typedef typename P::output_type
				output_type;
			P pred_;
			V value_;
			pred_ne( P pred, V value ): pred_(pred),value_(value) { }
			bool operator()( const input_type* obj ) const
			{
				return pred_(obj)!=value_;
			}
		};

		template <class P,class V>
		struct pred_gt: public expr_base<pred_gt<P,V> >
		{
			typedef typename P::input_type
				input_type;
			typedef typename P::output_type
				output_type;
			P pred_;
			V value_;
			pred_gt( P pred, V value ): pred_(pred),value_(value) { }
			bool operator()( const input_type* obj ) const
			{
				return pred_(obj)>value_;
			}
		};

		template <class P,class V>
		struct pred_lt: public expr_base<pred_lt<P,V> >
		{
			typedef typename P::input_type
				input_type;
			typedef typename P::output_type
				output_type;
			P pred_;
			V value_;
			pred_lt( P pred, V value ): pred_(pred),value_(value) { }
			bool operator()( const input_type* obj ) const
			{
				return pred_(obj)<value_;
			}
		};

		template <class P,class V>
		struct pred_ge: public expr_base<pred_ge<P,V> >
		{
			typedef typename P::input_type
				input_type;
			typedef typename P::output_type
				output_type;
			P pred_;
			V value_;
			pred_ge( P pred, V value ): pred_(pred),value_(value) { }
			bool operator()( const input_type* obj ) const
			{
				return pred_(obj)>=value_;
			}
		};

		template <class P,class V>
		struct pred_le: public expr_base<pred_le<P,V> >
		{
			typedef typename P::input_type
				input_type;
			typedef typename P::output_type
				output_type;
			P pred_;
			V value_;
			pred_le( P pred, V value ): pred_(pred),value_(value) { }
			bool operator()( const input_type* obj ) const
			{
				return pred_(obj)<=value_;
			}
		};

	}


	template <class L,class R>
	internal::pred_and<L,R>
	operator&&( const expr_base<L>& left, const expr_base<R>& right )
	{
		return internal::pred_and<L,R>( left.get(), right.get() );
	}

	template <class L,class R>
	internal::pred_or<L,R>
	operator||( const expr_base<L>& left, const expr_base<R>& right )
	{
		return internal::pred_or<L,R>( left.get(), right.get() );
	}

	template <class P>
	internal::pred_not<P>
	operator!( const expr_base<P>& pred )
	{
		return internal::pred_not<P>( pred.get() );
	}


	template <class P,class V>
	internal::pred_eq<P,V>
	operator==( const expr_base<P>& pred, const V& value )
	{
		return internal::pred_eq<P,V>( pred.get(), value );
	}										    

	template <class P,class V>
	internal::pred_eq<P,V>
	operator==( const V& value, const expr_base<P>& pred )
	{
		return internal::pred_eq<P,V>( pred.get(), value );
	}


	template <class P,class V>
	internal::pred_ne<P,V>
	operator!=( const expr_base<P>& pred, const V& value )
	{
		return internal::pred_ne<P,V>( pred.get(), value );
	}

	template <class P,class V>
	internal::pred_ne<P,V>
	operator!=( const V& value, const expr_base<P>& pred )
	{
		return internal::pred_ne<P,V>( pred.get(), value );
	}


	template <class P,class V>
	internal::pred_gt<P,V>
	operator>( const expr_base<P>& pred, V value )
	{
		return internal::pred_gt<P,V>( pred.get(), value );
	}

	template <class P,class V>
	internal::pred_gt<P,V>
	operator>( V value, const expr_base<P>& pred )
	{
		return internal::pred_gt<P,V>( pred.get(), value );
	}


	template <class P,class V>
	internal::pred_lt<P,V>
	operator<( const expr_base<P>& pred, const V& value )
	{
		return internal::pred_lt<P,V>( pred.get(), value );
	}

	template <class P,class V>
	internal::pred_lt<P,V>
	operator<( const V& value, const expr_base<P>& pred )
	{
		return internal::pred_lt<P,V>( pred.get(), value );
	}


	template <class P,class V>
	internal::pred_ge<P,V>
	operator>=( const expr_base<P>& pred, const V& value )
	{
		return internal::pred_ge<P,V>( pred.get(), value );
	}

	template <class P,class V>
	internal::pred_ge<P,V>
	operator>=( const V& value, const expr_base<P>& pred )
	{
		return internal::pred_ge<P,V>( pred.get(), value );
	}


	template <class P,class V>
	internal::pred_le<P,V>
	operator<=( const expr_base<P>& pred, const V& value )
	{
		return internal::pred_le<P,V>( pred.get(), value );
	}

	template <class P,class V>
	internal::pred_le<P,V>
	operator<=( const V& value, const expr_base<P>& pred )
	{
		return internal::pred_le<P,V>( pred.get(), value );
	}

}




//Function template X(container,functor)
//	Identical to for_each(container.begin(),container.end(),functor)

//Function template X(container,predicate,functor)
//	A predicate version of X(container,functor)

namespace scripting
{

	template <class C,class F>
	void X( C& container, F f )
	{
		for( typename C::const_iterator i=container.begin(); i!=container.end(); ++i )
			f( *i );
	}

	template <class C,class P,class F>
	void X( C& container, P pred, F f )
	{
		using namespace meta;
		for( typename C::const_iterator i=container.begin(); i!=container.end(); ++i )
			if( pred(*i) )
				f( ptr_cast<typename P::output_type>(&**i) );
	}

}
