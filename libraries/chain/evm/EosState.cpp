/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file EosState.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "EosState.h"

#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
#include <libdevcore/Assertions.h>
#include <libdevcore/TrieHash.h>
#include <libdevcore/TrieDB.h>

#include <libethcore/SealEngine.h>

#include <libevm/VMFactory.h>
#include <libethereum/BlockChain.h>
#include <libethereum/Block.h>
#include <libethereum/Defaults.h>
#include <libethereum/ExtVM.h>
#include <libethereum/TransactionQueue.h>
#include <libethereum/Executive.h>

