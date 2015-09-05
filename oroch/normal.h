// normal.h
//
// Copyright (c) 2015  Aleksey Demakov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef OROCH_NORMAL_H_
#define OROCH_NORMAL_H_

#include <memory>

namespace oroch {

template<typename T>
class normal_codec
{
public:
	using original_t = T;

	static constexpr size_t
	space(size_t nvalues)
	{
		return nvalues * sizeof(original_t);
	}

	template<typename DstIter, typename SrcIter>
	static bool
	encode(DstIter &dbegin, DstIter dend, SrcIter &sbegin, SrcIter send)
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		while (src < send) {
			if (dst + sizeof(original_t) > dend) {
				rc = false;
				break;
			}
			auto addr = std::addressof(*dst);
			*reinterpret_cast<original_t *>(addr) = *src++;
			dst += sizeof(original_t);
		}

		dbegin = dst;
		sbegin = src;
		return rc;
	}

	template<typename DstIter, typename SrcIter>
	static bool
	decode(DstIter &dbegin, DstIter dend, SrcIter &sbegin, SrcIter send)
	{
		bool rc = true;
		DstIter dst = dbegin;
		SrcIter src = sbegin;

		while (dst < dend) {
			if (src + sizeof(original_t) > send) {
				rc = false;
				break;
			}
			auto addr = std::addressof(*src);
			*dst++ = *reinterpret_cast<const original_t *>(addr);
			src += sizeof(original_t);
		}

		dbegin = dst;
		sbegin = src;
		return rc;
	}
};

} // namespace oroch

#endif /* OROCH_NORMAL_H_ */
