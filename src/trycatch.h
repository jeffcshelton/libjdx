#pragma once

#define TRY \
  int _error = 0; \
  if (1)

#define CATCH(e) \
  int e = _error; \
	_catch: \
    if (_error)

#define THROW(e) \
  _error = e; \
  goto _catch

