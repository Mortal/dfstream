template <typename T, int I, int N>
struct dataequal;

template <typename T, int N>
struct dataequal<T, 0, N> {
	inline static bool eq(const T *, const T *) {
		return true;
	}
};

template <>
struct dataequal<unsigned char, 3, 3> {
	typedef unsigned char T;
	static const int I = 3;
	static const int N = 3;


	inline static bool eq(const T * a, const T * b) {
		static const unsigned char mask[] = {255,255,255,0};
		static const int * imask = reinterpret_cast<const int *>(mask);
		return (*reinterpret_cast<const int *>(a) & *imask)
			== (*reinterpret_cast<const int *>(b) & *imask);
	}
};

template <typename T, int I, int N>
struct dataequal {
	inline static bool eq(const T * a, const T * b) {
		return a[N-I] == b[N-I] && dataequal<T, I-1, N>::eq(a, b);
	}
};

