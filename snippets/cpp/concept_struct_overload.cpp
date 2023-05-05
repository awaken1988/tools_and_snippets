template<typename T>
concept ConceptSigned = std::signed_integral<T>;

template<typename T>
concept ConceptUnsigned = std::unsigned_integral<T>;

template<typename T>
struct My {
    using my_type = T;
};

template<ConceptSigned T>
struct My<T> {
    using my_type = T;
};

template<ConceptUnsigned T>
struct My<T> {
    using my_type = T;
};
