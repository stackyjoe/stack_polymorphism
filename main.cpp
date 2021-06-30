#include "stack_polymorph.hpp"

#include <iostream>


struct animal {
    animal() = default;
    virtual ~animal() {}

    virtual animal * move_into(void * new_home) = 0;

    virtual void speak() = 0;
    virtual bool has_hair() = 0;
};

struct slug : public animal {
    slug() = delete;
    slug(bool inherited_boombox) : has_boombox(inherited_boombox) { }
    slug(slug &&) = default;
    slug(slug const &) = delete;
    ~slug() override = default;

    animal * move_into(void * new_home) override {
    	return new (new_home) slug(std::move(*this));
    }
    
    bool has_boombox;
    void speak() override {
        if(has_boombox) {
            std::cout << "boom bap" << std::endl;
        }
        else {
            std::cout << "slither" << std::endl;
        }
    }
    bool has_hair() override {
        return false;
    }
    
};

struct sheep : public animal {
    sheep() : shorn(false) { }
    sheep(sheep &&) = default;
    ~sheep() override = default;

    animal * move_into(void * new_home) override {
        return new (new_home) sheep(std::move(*this));
    }
    
    bool shorn;
    
    void speak() override {
        if(shorn) {
            std::cout << "it's chilly!" << std::endl;
        }
        else {
            std::cout << "baaaaa" << std::endl;
        }
    }
    bool has_hair() override {
        return not shorn;
    }
    
    void shave() {
        shorn = true;
    }
};

struct heap_slug : public animal {
    heap_slug() = delete;
    heap_slug(slug && sl) : slug_in_heap(std::make_unique<slug>(std::move(sl))) { }
    //heap_slug(slug const & sl) : slug_in_heap(std::make_unique<slug>(sl)) { }
    
    heap_slug(heap_slug &&other) : slug_in_heap{std::move(other.slug_in_heap)} {}
    heap_slug(heap_slug const &) = delete;
    
    ~heap_slug() override = default;
    
    std::unique_ptr<slug> slug_in_heap;

    animal * move_into(void * new_home) override {
        return new (new_home) heap_slug(std::move(*this));
    }
    
    void speak() override {
        slug_in_heap->speak();
    }
    
    bool has_hair() override {
        return slug_in_heap->has_hair();
    }
};

int main() {
    using polymorph = stack_polymorph<animal, heap_slug, sheep>;
    
    polymorph a;
    a.construct_in_place<sheep>();
    
    sheep bob;
    bob.shave();
    polymorph b { std::move(bob) };
    
    slug clarice(true);
    heap_slug clarice_in_heap { std::move(clarice) };
    
    polymorph c;
    c.construct_in_place<heap_slug>(std::move(clarice_in_heap));
    
    b->speak();
    a->speak();
    c->speak();
    
    polymorph d;
    d = std::move(c);
    d->speak();
}
