#include "strings.h"

//Isolate token
CHAR16 *EFIAPI SuIsolateToken(CHAR16 *pCommand, UINT32 tokenNr)
{
	CHAR16 *retVal = NULL;
	BOOLEAN tokenHit = FALSE;
	UINT64 len = StrLen(pCommand), tokenLen = 0, hitNr = 0;

	if (!(retVal = AllocateZeroPool((len + 1) * sizeof(WCHAR))))
	{
		Print(L"Allocation failed\n");
		goto Done;
	}

	for (UINT64 i = 0; i < len; i++)
	{
		if (tokenNr == hitNr && pCommand[i] != ' ')
		{
			tokenHit = TRUE;
			retVal[tokenLen] = pCommand[i];
			tokenLen++;
		}
		else if (tokenNr == hitNr && pCommand[i] == ' ')
		{
			break;
		}
		else if (pCommand[i] == ' ')
		{
			hitNr++;
		}
	}

	retVal[tokenLen] = '\0';

Done:

	if (!tokenHit)
	{
		FreePool(retVal);
		retVal = NULL;
	}

	return retVal;
}

CHAR8 *EFIAPI SuUnicodeToAscii(CHAR16 *str)
{
	CHAR8 *retVal = NULL;
	UINTN strLen = StrLen(str);

	if(!(retVal = AllocatePool(strLen + 1)))
		goto Done;

	for (UINTN i = 0; i < strLen; i++)
	{
		retVal[i] = (CHAR8)str[i];
	}

	retVal[strLen] = '\0';

Done:

	return retVal;
}

//Compare token
BOOLEAN EFIAPI SuCompareToken(CHAR16 *pCmd, CHAR16 *pToken, UINT32 tokenNr)
{
	BOOLEAN retVal = FALSE, newToken = TRUE;
	UINT64 len = StrLen(pCmd), lenToken = StrLen(pToken), token = 0;

	if (len < lenToken)
		return FALSE;

	for (UINT64 i = 0; (i < len - lenToken + 1); i++)
	{
		if (newToken && tokenNr == token)
		{
			if (!CompareMem(&pCmd[i], pToken, lenToken * sizeof(CHAR16)))
			{
				retVal = TRUE;
			}

			break;
		}
	
		if (pCmd[i] == ' ')
		{
			newToken = TRUE;
			token++;
		}
		else
			newToken = FALSE;
	}

	return retVal;
}

//Check if command only contains valid symbols
BOOLEAN EFIAPI SuValidateCommand(CHAR16 *pCommand)
{
	BOOLEAN retVal = TRUE;

	for (UINT64 i = 0; pCommand[i]; i++)
	{
		if (pCommand[i] < 32 || pCommand[i] > 125)
		{
			retVal = FALSE;
			break;
		}
	}

	return retVal;
}

BOOLEAN EFIAPI SuIsNumerical(CHAR16 *str)
{
	BOOLEAN retVal = TRUE;

	for (UINT32 i = 0; str[i] && i < 5; i++)
	{
		if (str[i] < 48 || str[i] > 57)
		{
			retVal = FALSE;
			break;
		}
	}

	return retVal;
}

//hash unicode string
UINT32 EFIAPI SuHashString(CHAR16 *str)
{
	unsigned long hash = 5381;
	int c;

	if(str)
		while (c = *str++)
			hash = ((hash << 5) + hash) + c;

	return hash;
}

int EFIAPI isspacec(int c)
{
	return ((c == ' ') || (c == '\n') || (c == '\t'));
}

int EFIAPI isdigitc(int c)
{
	return (c >= '0' && c <= '9' ? 1 : 0);
}

int EFIAPI isalphac(int c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ? 1 : 0);
}

int EFIAPI isupperc(int c)
{
	return (c >= 'A' && c <= 'Z');
}

unsigned long EFIAPI strtoulc(const char *nptr, char **endptr, register int base)
{
	register const char *s = nptr;
	register unsigned long acc;
	register int c;
	register unsigned long cutoff;
	register int neg = 0, any, cutlim;
	
	do 
	{
		c = *s++;
	} while (isspacec(c));

	if (c == '-') 
	{
		neg = 1;
		c = *s++;
	}
	else if (c == '+')
		c = *s++;

	if ((base == 0 || base == 16) &&
		c == '0' && (*s == 'x' || *s == 'X')) 
	{
		c = s[1];
		s += 2;
		base = 16;
	}

	if (base == 0)
		base = c == '0' ? 8 : 10;

	cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
	cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;

	for (acc = 0, any = 0;; c = *s++) 
	{
		if (isdigitc(c))
			c -= '0';
		else if (isalphac(c))
			c -= isupperc(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else
		{
			any = 1;
			acc *= base;
			acc += c;
		}
	}

	if (any < 0)
	{
		acc = ULONG_MAX;
	}
	else if (neg)
		0;
	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);

	return (acc);
}


