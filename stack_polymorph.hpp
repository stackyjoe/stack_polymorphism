#include <memory>
#include <type_traits>


template<typename Base, class ...Derivations>
class stack_polymorph {
    static_assert( (std::is_base_of_v<Base, Derivations> && ... ),
                   "All classes passed as template parameter to stack_polymorph must be derived from the first template parameter.");
    static_assert( (std::has_virtual_destructor_v<Base> && (std::is_destructible_v<Derivations> && ...)),
                   "First template parameter, Base,  in stack_polymorph must have a virtual destructor, and the subsequent types must be destructible.");
private:
    std::aligned_union_t<1,Derivations...> storage;
    Base * valid_ptr;

void move_into(stack_polymorph &other) {
        static_assert(std::is_invocable_r_v<Base*,decltype(&Base::move_into), Base, decltype(std::addressof(storage))>,
	              "move_into cannot be defined because the template arguments to stack_polymorph do not define it.");

	other.clear();

        if(valid_ptr) {
	    other.valid_ptr = valid_ptr->move_into(std::addressof(other.storage));
	}

	clear();
}

void copy_into(stack_polymorph &other) const {
        static_assert(std::is_invocable_r_v<Base*,decltype(&Base::copy_into), const Base, decltype(std::addressof(storage))>,
	              "copy_into cannot be defined because the template arguments to stack_polymorph do not define it.");

	other.clear();

        if(valid_ptr) {
	    other.valid_ptr = valid_ptr->copy_into(std::addressof(other.storage));
	}

	clear();

}


public:
    bool is_clear() const noexcept {
        return valid_ptr == nullptr;
    }

    void clear() {
	if(valid_ptr)
	    valid_ptr->Base::~Base();
	valid_ptr = nullptr;
    }

    stack_polymorph() : storage(), valid_ptr(nullptr) { }
    
    ~stack_polymorph() {
    	clear();
    }
    stack_polymorph(stack_polymorph const &other) {
        other.copy_into(*this);
    }

    stack_polymorph & operator=(stack_polymorph const &other) {
    	other.copy_into(*this);
	return *this;
    }

    stack_polymorph(stack_polymorph &&other) {
        other.move_into(*this);
    }

    stack_polymorph & operator=(stack_polymorph &&other) {
    	other.move_into(*this);
	return *this;
    }
    

    Base * get() noexcept {
        return valid_ptr;
    }

    Base const * get() const noexcept {
	return valid_ptr;
    }

    Base & operator*() noexcept {
        return *valid_ptr;
    }

    Base * operator->() noexcept {
	return valid_ptr;
    }

    template<typename Derived, typename ...ConstructorArgs>
    void construct_in_place(ConstructorArgs ...args) {
        static_assert( std::is_same<Base,Derived>::value || (std::is_same<Derived, Derivations>::value || ... ),
                       "Only a template parameter for a given instance of stack_polymorph may be used as a template parameter for the same instance's construct_in_place function.");
        static_assert( (std::is_constructible<Derived, ConstructorArgs...>::value),
                       "Arguments to stack_polymorph construct_in_place must be valid arguments to a constructor for its template parameter.");
        clear();

        valid_ptr = new (std::addressof(storage)) Derived(std::forward<ConstructorArgs>(args) ...);
    }

    template<typename Derived>
    stack_polymorph(Derived && instance)
        : storage(),
          valid_ptr(new(std::addressof(storage)) Derived(std::move(instance))) {
        static_assert( std::is_same<Base,Derived>::value || (std::is_same<Derived, Derivations>::value || ... ),
                       "Only a template parameter for a given instance of stack_polymorph may be used as a template parameter for the same instance's construct_in_place function.");
        static_assert( std::is_move_constructible<Derived>::value,
                       "stack_polymorph only defines a constructor with rvalue reference argument for movable types");
    }


};

