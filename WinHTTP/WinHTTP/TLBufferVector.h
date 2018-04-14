#pragma once
#include<assert.h>
typedef unsigned char BYTE;
#define TLASSERT assert


template<class CELL = BYTE, bool CheckSize_ = true>
class TLBufferVector
{
public:
	TLBufferVector(size_t cCell = 0);
	~TLBufferVector(void);

	size_t GetCount() const
	{
		return *(size_t *)GetBuffer();
	}

	CELL *Ptr(size_t offset, size_t cCellToVisit);
	const CELL *Ptr(size_t offset, size_t cCellToVisit) const;

	void Reset();

	void CopyFrom(size_t offsetInDest, const CELL *pSrc, size_t size);
	void CopyFrom(const CELL *pSrc, size_t size)
	{
		CopyFrom(0, pSrc, size);
	}

	void ReplacePos(size_t offsetStartInDest, size_t offsetEndInDest, const CELL *pSrc, size_t cCell);

protected:

#ifdef _DEBUG
	const static BYTE c_chDogtag = 0xcd;
	const static int c_cbDogtag = 4;
	const static int c_mcUnit = 16 * 1024 * 1024;
#else
	const static TCHAR c_chDogtag = 0;
	const static int c_cbDogtag = 0;
#endif

	static size_t GetBufferSize(size_t cCell)
	{
		return cCell * sizeof(CELL) + sizeof(size_t) + 2 * c_cbDogtag;
	}

	BYTE *GetBuffer() const
	{
		if (m_bIsInStack)
		{
			return m_buf;
		}
		else
		{
			return m_pBuffer;
		}
	}
	BYTE *m_pBuffer;

#ifdef _DEBUG
	const CELL *m_sz;
#endif

	void SetCount(size_t cCell)
	{
		*(size_t *)GetBuffer() = cCell;
	}

	void Expand(size_t cCell);
	void FillDogtag(size_t start, size_t end)
	{
#ifdef _DEBUG
		memset(GetBuffer() + start, c_chDogtag, end - start);
#endif
	}

#ifdef _DEBUG
	bool DataValid() const; // 校验数据
#endif

	static const int STACKSIZE = 270;
	mutable BYTE m_buf[STACKSIZE];
	short m_cbStack;
	bool m_bIsInStack;

	void *Malloc(size_t cb)
	{
		if (!m_bIsInStack)
		{
			return malloc(cb);
		}
		if (cb > STACKSIZE)
		{
			m_bIsInStack = false;
			return malloc(cb);
		}
		m_cbStack = (short)cb;
		return m_buf;
	}

	void *Realloc(void *p, size_t cb)
	{
		if (!m_bIsInStack)
		{
			return realloc(p, cb);
		}
		TLASSERT(p == m_buf);
		if (cb > STACKSIZE)
		{
			m_bIsInStack = false;
			void *ptr = malloc(cb);
			memcpy(ptr, m_buf, m_cbStack);
			return ptr;
		}
		m_cbStack = (short)cb;
		return m_buf;
	}

	void Free(void *p)
	{
		if (p != m_buf)
		{
			free(p);
		}
	}
};

template<class CELL, bool CheckSize_>
TLBufferVector<CELL, CheckSize_>::TLBufferVector(size_t cCell)
	: m_pBuffer(NULL)
{
	assert(cCell >= 0);
	m_bIsInStack = true;
	m_cbStack = 0;
	size_t cbBuffer = GetBufferSize(cCell);
	m_pBuffer = (BYTE *)Malloc(cbBuffer);
	FillDogtag(0, cbBuffer); // 填充垃圾数据（含狗牌）
	SetCount(cCell);
}

template<class CELL, bool CheckSize_>
TLBufferVector<CELL, CheckSize_>::~TLBufferVector(void)
{
	assert(GetBuffer() != NULL);
	assert(DataValid());
	Free(GetBuffer());
}

#ifdef _DEBUG
template<class CELL, bool CheckSize_>
bool TLBufferVector<CELL, CheckSize_>::DataValid() const // 检查狗牌
{
	if (GetBuffer() == NULL)
	{
		return false;
	}

	// 检查size取值是否合理
	size_t cCell = GetCount();
	if (cCell < 0 || (CheckSize_ && cCell > c_mcUnit))
	{
		return false;
	}

	// 检查狗牌
	const BYTE *p1 = GetBuffer() + sizeof(size_t);
	const BYTE *p2 = p1 + c_cbDogtag + GetCount() * sizeof(CELL);
	for (size_t i = 0; i < c_cbDogtag; i++)
	{
		if (p1[i] != c_chDogtag)
		{
			return false;
		}
		if (p2[i] != c_chDogtag)
		{
			return false;
		}
	}
	return true;
}
#endif

template<class CELL, bool CheckSize_>
void TLBufferVector<CELL, CheckSize_>::Expand(size_t cCell)
{
	TLASSERT(DataValid());
	size_t cNew = cCell;
	TLASSERT(cNew >= 0);

	if (CheckSize_)
	{
		TLASSERT(cNew <= c_mcUnit);
	}

	size_t cOld = GetCount();
	if (cNew <= cOld)
	{
		return;
	}

	size_t cExpected = ((cOld * 2 + 0x0010) & 0xFFF0);
	if (cNew < cExpected)
	{
		cNew = cExpected;
	}
	TLASSERT(cNew > cOld);
	m_pBuffer = (BYTE *)Realloc(GetBuffer(), GetBufferSize(cNew));
	FillDogtag(GetBufferSize(cOld) - c_cbDogtag, GetBufferSize(cNew));
	SetCount(cNew);
#ifdef _DEBUG
	m_sz = (const CELL *)Ptr(0, cNew);
#endif
	TLASSERT(DataValid());
}

template<class CELL, bool CheckSize_>
CELL *TLBufferVector<CELL, CheckSize_>::Ptr(size_t offset, size_t cCell)
{
	TLASSERT(DataValid());
	Expand(offset + cCell);
	TLASSERT(GetCount() >= offset + cCell);
	return (CELL *)(GetBuffer() + sizeof(size_t) + c_cbDogtag + offset * sizeof(CELL));
}

template<class CELL, bool CheckSize_>
const CELL *TLBufferVector<CELL, CheckSize_>::Ptr(size_t offset, size_t cCell) const
{
	TLASSERT(DataValid());
	TLASSERT(GetCount() >= offset + cCell);
	return (const CELL *)(GetBuffer() + sizeof(size_t) + c_cbDogtag + offset * sizeof(CELL));
}

template<class CELL, bool CheckSize_>
void TLBufferVector<CELL, CheckSize_>::Reset()
{
	TLASSERT(DataValid());
	TLASSERT(GetBuffer() != NULL);
	size_t cNew = 0;
	size_t cOld = GetCount();
	TLASSERT(cNew <= cOld);
	FillDogtag(GetBufferSize(cNew) - c_cbDogtag, GetBufferSize(cOld));
	m_pBuffer = (BYTE *)Realloc(GetBuffer(), GetBufferSize(cNew));
	SetCount(cNew);
	TLASSERT(DataValid());
}

template<class CELL, bool CheckSize_>
void TLBufferVector<CELL, CheckSize_>::CopyFrom(size_t offset, const CELL *pSrc, size_t cCell)
{
	CELL *pDest = Ptr(offset, cCell);
	memcpy(pDest, pSrc, cCell * sizeof(CELL));
}

template<class CELL, bool CheckSize_>
void TLBufferVector<CELL, CheckSize_>::ReplacePos(size_t start, size_t end, const CELL *pSrc, size_t cCell)
{
	size_t nDstLen = end - start;
	size_t nLeft = GetCount() - end;
	CELL *pDstStart = Ptr(start, nDstLen);
	CELL *pDstEnd = Ptr(end, GetCount() - end);

	if (nDstLen >= cCell)
	{
		int nDiff = (int)(nDstLen - cCell);
		memmove(pDstStart + cCell, pDstEnd, nLeft * sizeof(CELL));
		memcpy(pDstStart, pSrc, cCell * sizeof(CELL));
	}
	else if (nDstLen < cCell)
	{
		// expand
		int nDiff = (int)(cCell - nDstLen);
		Expand(GetCount() + nDiff);
		// pointer will be changed after Expand
		pDstStart = Ptr(start, nDstLen);
		pDstEnd = Ptr(end, GetCount() - end);
		memmove(pDstEnd + nDiff, pDstEnd, nLeft * sizeof(CELL));
		memcpy(pDstStart, pSrc, cCell * sizeof(CELL));
	}
}

typedef TLBufferVector<BYTE, false> TLByteBufferVector;

