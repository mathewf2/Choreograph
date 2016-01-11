/*
 * Copyright (c) 2014—2016 David Wicks, sansumbrella.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include <vector>
#include "Phrase.hpp"
#include "UnitBezier.h"
#include "TimeType.h"

namespace choreograph
{

template <typename T>
class Channel
{
public:
  struct Key {
    Key(T value, Time time):
      value(value),
      time(time)
    {}
    T    value;
    Time time;
  };

  Channel() = default;

  /// Return the value of the channel at a given time.
  T value(Time at_time) const;
  /// Return the interpolated value between two keys.
  /// If you keep track of keys yourself, this is faster.
  T interpolatedValue(size_t curve_index, Time at_time) const;

  /// Returns the index of the curve for the current time.
  /// The curve lies between two keys.
  size_t index(Time at_time) const;

  /// Append a key to the list of keys, positioned at \offset time after the previous key.
  Channel& appendKeyAfter(T value, Time offset) {
    if (! _keys.empty()) {
      _curves.emplace_back();
    }
    _keys.emplace_back(value, duration() + offset);
    return *this;
  }
  /// Insert a key at the given time.
  Channel& insertKey(T value, Time at_time);

  Time     duration() const { return _keys.empty() ? 0 : _keys.back().time; }

private:
  std::vector<Key>               _keys;
  // may make polymorphic in future (like phrases are). Basically just an ease fn.
  // Compare std::function<float (float)> for max flexibility against custom class.
  std::vector<BezierInterpolant> _curves;
};

#pragma mark - Channel Template Implementation

template <typename T>
size_t Channel<T>::index(Time at_time) const {
  if( at_time <= 0 ) {
    return 0;
  }
  else if ( at_time >= this->duration() ) {
    return _curves.size() - 1;
  }

  for (auto i = 0; i < _keys.size() - 1; i += 1) {
    auto &a = _keys[i], &b = _keys[i + 1];
    if (a.time <= at_time && b.time >= at_time) {
      return i;
    }
  }
  return _curves.size() - 1;
}

template <typename T>
T Channel<T>::value(Time at_time) const {
  if (at_time >= duration()) {
    return _keys.back().value;
  }
  else if (at_time <= 0.0) {
    return _keys.front().value;
  }

  return interpolatedValue(index(at_time), at_time);
}

template <typename T>
T Channel<T>::interpolatedValue(size_t curve_index, Time at_time) const {
  auto &a = _keys[curve_index];
  auto &b = _keys[curve_index + 1];
  auto &c = _curves[curve_index];

  auto x = (at_time - a.time) / (b.time - a.time);
  auto t = c.solve(x, std::numeric_limits<float>::epsilon());
  return lerpT(a.value, b.value, t);
}

template <typename T>
Channel<T>& Channel<T>::insertKey(T value, Time at_time) {
  if (_keys.empty()) {
    _keys.emplace_back(value, at_time);
  }
  else {
    _keys.insert(_keys.begin() + (index(at_time) + 1), {value, at_time});
  }

  return *this;
}

} // namepsace choreograph
