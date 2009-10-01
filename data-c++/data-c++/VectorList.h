#ifndef DATA_CPP_VECTORLIST_H
#define DATA_CPP_VECTORLIST_H

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cassert>

namespace data_cpp {
    /** A VectorList is similar to the Java ArrayList:
     *  It is a data structure designed for random access
     *  and appending data.
     *
     *  In this implementation, it is a vector of vectors.
     *  The secondary vectors have a fixed size \c binSize.
     *  All allocation is done using malloc().
     *
     *  The secondary vectors are called bins. At any time,
     *  all bins but the last are filled completely, while
     *  new elements are filled into the last bin.
     *
     *  The Alignment template parameter can be choosen to
     *  any value != 1 to ensure values are aligned at these
     *  boundaries. */
    template <typename Type, unsigned int Alignment = 0x1>
    class VectorList {
      private:
        Type **array, **allocated;
        const int binSize;
        int arrayBins, currentBin, indexInBin;

        void makeAlignedBin(int size) {
            void *fresh = malloc(size+(Alignment-1));
            size_t addr = reinterpret_cast<size_t>(fresh) & ~(Alignment-1);
            if ( addr != reinterpret_cast<size_t>(fresh) )
                addr += Alignment;
            void *aligned = reinterpret_cast<void*>(addr);

            allocated[currentBin] = (Type*)fresh;
            array[currentBin] = (Type*)aligned;
        }
        void init() {
            size_t size = sizeof(Type*) * arrayBins;
            array = (Type**)malloc(size);
            allocated = (Type**)malloc(size);

            currentBin = indexInBin = 0;

            makeAlignedBin( sizeof(Type) * binSize );
            for (int i = 1; i < arrayBins; i++)
                array[i] = NULL;
        }
        static void enlarge(Type**& array, int old_size, int new_size) {
            Type **newArray = (Type**)malloc
                (sizeof(Type*) * new_size);
            memcpy(newArray, array, sizeof(Type*) * old_size);
            memset(newArray+old_size, 0, 
                             sizeof(Type*) * (new_size-old_size) );
            free(array);
            array = newArray;
        }
        void makeNewBin() {
            currentBin++;
            if (currentBin == arrayBins) {
                int newArrayBins = 2 * arrayBins;
                enlarge(array, arrayBins, newArrayBins);
                enlarge(allocated, arrayBins, newArrayBins);
                arrayBins = newArrayBins;
            }

            if (array[currentBin] == NULL)
                makeAlignedBin( sizeof(Type) * binSize );
            
            indexInBin = 0;
        }
        void dealloc() {
            for (int i = 0; i < arrayBins; i++)
                if ( array[i] != NULL ) {
                    for (int e = 0; e < sizeOfBin(i); e++)
                        array[i][e].~Type();
                    free(allocated[i]);
                    array[i] = NULL;
                    arrayBins = 0;
                } else
                    break;

            free(array);
            free(allocated);
        }

      public:
        /** Constructs an empty VectorList */
        VectorList() : binSize(1000), arrayBins(1000) { init(); }
        /** Destructor. */
        ~VectorList() { dealloc(); }

        VectorList (const VectorList<Type>& copy_from) 
        : binSize(copy_from.binSize)
        {
            arrayBins = 0; array = NULL;
            (*this) = copy_from;
        }
        VectorList<Type>& operator=(const VectorList<Type>& from)
        {
            if ( arrayBins < from.currentBin ) {
                dealloc();
                arrayBins = from.arrayBins;
                init();
            }
            while ( currentBin < from.currentBin )
                makeNewBin();

            for (int bin = 0; bin < from.binNumber(); bin++)
                for (int el = 0; el < from.sizeOfBin(bin); el++)
                    new (array[bin]+el) Type(from[bin*binSize+el]);

            currentBin = from.currentBin;
            indexInBin = from.indexInBin;
            return *this;
        }


        inline const Type& operator[](int index) const {
            assert( index <= currentBin * binSize + indexInBin );
            return array[ index / binSize ][ index % binSize ];
        }
        /** Returns the number of independent bins that could 
         *  be accessed with getBin(). */
        inline int binNumber() const { return currentBin+1; }
        /** Get a pointer to the elements in bin number \c num. */
        inline const Type* getBin(int num) const
            { return array[num]; }
        /** Get a pointer to the elements in bin number \c num. */
        inline Type* getBin(int num) { return array[num]; }
        /** Get the number of elements in the bin number \c num. */
        inline int sizeOfBin(int num) const 
            { return (num < currentBin) ? binSize : indexInBin; }

        /** Add a new element to the VectorList. */
        inline void push_back(const Type &fit) { 
            new(allocate()) Type(fit);
        } 

        /** Allocate space for a new element, but do not initialize. */
        inline Type* allocate() { 
            if (indexInBin == binSize) 
                makeNewBin();
            Type* rv = array[currentBin] + indexInBin;
            indexInBin++;
            return rv;
        }

        /** Returns the total number of elements in the VectorList. */
        inline unsigned int size() 
            { return binSize * currentBin + indexInBin; }
         
        /** Removes all elements from the VectorList. */
        inline void clear()
            { currentBin = indexInBin = 0; }
         
        /** Returns a reference to the first element in the
         *  VectorList. */
        inline Type& front()
            { return array[0][0]; }
        /** Returns a reference to the last element in the
         *  VectorList. */
        inline Type& back()
            { if (indexInBin == 0)
                return array[currentBin-1][binSize-1];
              else
                return array[currentBin][indexInBin-1]; 
            }

        /** Standard STL iterator. */
        class const_iterator {
          private:
            friend class VectorList;
            Type **array;
            Type *run;
            int runCount, binSize;

          public:
            const_iterator(int binSize) : binSize(binSize) {}
            inline const Type &operator*() { return *run; }
            inline const Type *operator->() { return run; }

            inline bool operator!=(const const_iterator &other)
                { return ! ((*this) == other); }
            inline bool operator==(const const_iterator &other)
                { return array == other.array && run == other.run; }

            inline const_iterator& operator++() 
                { return operator++(0); }

            inline const_iterator& operator++(int)
                { run++; runCount++; 
                  if (runCount == binSize) 
                    { run = *( ++array ); runCount = 0; }
                  return *this;
                }

        };
        friend class const_iterator;

        /** Returns an iterator to the first element. */
        inline const_iterator begin() const
        { 
            const_iterator it(binSize);
            it.array = array;
            it.run = array[0];
            it.runCount = 0;
            return it;
        }

        /** Returns an iterator past the last element. */
        inline const_iterator end() const
        { 
            const_iterator it(binSize);
            int cB = currentBin + indexInBin / binSize, i = indexInBin % binSize;
            it.array = array + cB;
            it.run = array[cB] + i;
            it.runCount = i;
            return it;
        }

    };
}

#endif
