//
// Created by vortox on 24/3/20.
//

#ifndef BPTREE_ALGORITHM_H
#define BPTREE_ALGORITHM_H
#include <iterator>

namespace ds{

    template< class ForwardIt, class T >
    ForwardIt lower_bound( ForwardIt first, ForwardIt last, const T& value ) {
        while (first<last){
            auto mid=first;
            std::advance(mid,(last-first)/2);
            if(*mid<value)first=mid+1;
            else last=mid;
        }
        return first;
    }

    template< class ForwardIt, class T >
    ForwardIt upper_bound( ForwardIt first, ForwardIt last, const T& value ){
        while (first<last){
            auto mid=first;
            std::advance(mid,(last-first)/2);
            if(*mid<value||*mid==value)first=mid+1;
            else last=mid;
        }
        return first;
    }

    template< class InputIt, class OutputIt >
    OutputIt move( InputIt first, InputIt last, OutputIt d_first ){
        while (first != last)
            *d_first++ = std::move(*first++);
        return d_first;
    }

    template< class BidirIt1, class BidirIt2 >
    BidirIt2 move_backward(BidirIt1 first,BidirIt1 last,BidirIt2 d_last){
        while (first != last)
            *(--d_last) = std::move(*(--last));
        return d_last;
    }

}
#endif //BPTREE_ALGORITHM_H
