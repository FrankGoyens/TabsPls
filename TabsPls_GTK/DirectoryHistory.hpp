#pragma once

struct DirectoryHistory
{
	virtual ~DirectoryHistory() = default;
	
	virtual void RequestPreviousDirectory() const = 0;
	virtual void RequestNextDirectory() const = 0;
};


