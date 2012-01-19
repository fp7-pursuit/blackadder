/*
 * Copyright (C) 2011  Christos Tsilopoulos, Mobile Multimedia Laboratory, 
 * Athens University of Economics and Business 
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of
 * the BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

package eu.pursuit.client;

import java.nio.ByteBuffer;
import java.util.concurrent.TimeUnit;

import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.Event;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.Publication;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;
import eu.pursuit.core.StrategyOptions;
import eu.pursuit.core.Util;

public class BlackAdderClient {

	public static final byte[] EMPTY_ARRAY = new byte[0];

	private final BlackadderWrapper baWrapper;
	private boolean closed;
	private static BlackAdderClient instance = null;

	public static BlackAdderClient getInstance() {
		if (instance == null) {
			instance = new BlackAdderClient();
		}

		return instance;
	}

	private BlackAdderClient() {
		baWrapper = BlackadderWrapper.getWrapper();
		closed = false;
	}

	public void publishRootScope(ByteIdentifier scope, Strategy strategy,
			StrategyOptions options) {
		publishScope(scope, null, strategy, options);
	}
	
	public void publishScope(ByteIdentifier scope, ScopeID prefixScope,	Strategy strategy){
		publishScope(scope, prefixScope, strategy, null);
	}

	public void publishScope(ByteIdentifier scope, ScopeID prefixScope,
			Strategy strategy, StrategyOptions options) {
		byte[] scopeStr = scope == null ? EMPTY_ARRAY : scope.getId();
		byte[] prefixStr = prefixScope == null ? EMPTY_ARRAY : prefixScope
				.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte[] opt_arr = null;

		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}

		baWrapper.publishScope(scopeStr, prefixStr, strategyValue, opt_arr);
	}
	
	public void republishScope(ScopeID scope, ScopeID superScope, Strategy strategy){
		republishScope(scope, superScope, strategy, null);
	}

	public void republishScope(ScopeID scope, ScopeID superScope,
			Strategy strategy, StrategyOptions options) {
		Util.checkNull(scope);
		Util.checkNull(superScope);

		byte[] scopeStr = scope == null ? EMPTY_ARRAY : scope.toByteArray();
		byte[] prefixStr = superScope == null ? EMPTY_ARRAY : superScope
				.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte[] opt_arr = null;

		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}

		baWrapper.publishScope(scopeStr, prefixStr, strategyValue, opt_arr);
	}
	
	public void publishItem(ItemName name, Strategy strategy){
		publishItem(name, strategy, null);
	}

	public void publishItem(ItemName name, Strategy strategy,
			StrategyOptions options) {

		byte[] ridStr = name.getRendezvousId() == null ? EMPTY_ARRAY : name
				.getRendezvousId().getId();
		byte[] scopeStr = name.getScopeId() == null ? EMPTY_ARRAY : name
				.getScopeId().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] opt_arr = null;
		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}
		
		baWrapper.publishItem(ridStr, scopeStr, strategyValue, opt_arr);
	}
	
	public void republishItem(ItemName name, ScopeID scope, Strategy strategy) {
		republishItem(name, scope, strategy, null);
	}

	public void republishItem(ItemName name, ScopeID scope, Strategy strategy,
			StrategyOptions options) {
		Util.checkNull(name);
		Util.checkNull(name.getRendezvousId(), "no rid found in name");
		Util.checkNull(name.getScopeId(), "no scope found in name");
		Util.checkNull(scope);

		byte[] nameStr = name.toByteArray();
		byte[] scopeStr = scope.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] opt_arr = null;
		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}
		
		baWrapper.publishItem(nameStr, scopeStr, strategyValue, opt_arr);
	}
	
	public void unpublishScope(ScopeID scope, ScopeID prefixScope, Strategy strategy){
		unpublishScope(scope, prefixScope, strategy, null);
	}

	public void unpublishScope(ScopeID scope, ScopeID prefixScope,
			Strategy strategy, StrategyOptions options) {
		byte[] scopeStr = scope == null ? EMPTY_ARRAY : scope.toByteArray();
		byte[] prefixStr = prefixScope == null ? EMPTY_ARRAY : prefixScope
				.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] opt_arr = null;
		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}

		baWrapper.unpublishScope(scopeStr, prefixStr, strategyValue, opt_arr);
	}
	
	public void unpublishItem(ItemName name, Strategy strategy){
		unpublishItem(name, strategy, null);
	}

	public void unpublishItem(ItemName name, Strategy strategy,
			 StrategyOptions options) {
		Util.checkNull(name);
		Util.checkNull(name.getRendezvousId(), "no rid found in name");
		Util.checkNull(name.getScopeId(), "no scope found in name");

		byte[] ridStr = name.getRendezvousId() == null ? EMPTY_ARRAY : name
				.getRendezvousId().getId();
		byte[] scopeStr = name.getScopeId() == null ? EMPTY_ARRAY : name
				.getScopeId().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] opt_arr = null;
		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}

		baWrapper.unpublishItem(ridStr, scopeStr, strategyValue, opt_arr);
	}
	
	public void subscribeScope(ScopeID scope, ScopeID prefixScope, Strategy strategy){
		subscribeScope(scope, prefixScope, strategy, null);
	}

	public void subscribeScope(ScopeID scope, ScopeID prefixScope,
			Strategy strategy, StrategyOptions options) {
		byte[] scopeStr = scope == null ? EMPTY_ARRAY : scope.toByteArray();
		byte[] prefixStr = prefixScope == null ? EMPTY_ARRAY : prefixScope
				.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] opt_arr = null;
		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}
		baWrapper.subscribeScope(scopeStr, prefixStr, strategyValue, opt_arr);
	}
	
	public void subscribeItem(ItemName name, Strategy strategy) {
		subscribeItem(name, strategy, null);
	}

	public void subscribeItem(ItemName name, Strategy strategy,
			StrategyOptions options) {
		Util.checkNull(name);
		Util.checkNull(name.getRendezvousId(), "no rid found in name");
		Util.checkNull(name.getScopeId(), "no scope found in name");

		byte[] ridStr = name.getRendezvousId() == null ? EMPTY_ARRAY : name
				.getRendezvousId().getId();
		byte[] scopeStr = name.getScopeId() == null ? EMPTY_ARRAY : name
				.getScopeId().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] opt_arr = null;
		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}

		baWrapper.subscribeItem(ridStr, scopeStr, strategyValue, opt_arr);
	}
	
	public void unsubscribeScope(ScopeID scope, ScopeID prefixScope, Strategy strategy){
		unsubscribeScope(scope, prefixScope, strategy, null);
	}

	public void unsubscribeScope(ScopeID scope, ScopeID prefixScope,
			Strategy strategy, StrategyOptions options) {
		byte[] scopeStr = scope == null ? EMPTY_ARRAY : scope.toByteArray();
		byte[] prefixStr = prefixScope == null ? EMPTY_ARRAY : prefixScope
				.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] opt_arr = null;
		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}

		baWrapper.unsubscribeScope(scopeStr, prefixStr, strategyValue, opt_arr);
	}
	
	public void unsubscribeItem(ItemName name, Strategy strategy){
		unsubscribeItem(name, strategy, null);
	}

	public void unsubscribeItem(ItemName name, Strategy strategy,
			StrategyOptions options) {
		Util.checkNull(name);
		Util.checkNull(name.getRendezvousId(), "no rid found in name");
		Util.checkNull(name.getScopeId(), "no scope found in name");

		byte[] ridStr = name.getRendezvousId() == null ? EMPTY_ARRAY : name
				.getRendezvousId().getId();
		byte[] scopeStr = name.getScopeId() == null ? EMPTY_ARRAY : name
				.getScopeId().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] opt_arr = null;
		if (options != null && options.getSerializedSize() > 0) {
			opt_arr = options.toByteArray();
		}

		baWrapper.unsubscribeItem(ridStr, scopeStr, strategyValue, opt_arr);
	}
	
	public void publishData(Publication publication, Strategy strategy) {
		publishData(publication, strategy, null);
	}		

	public void publishData(Publication publication, Strategy strategy,
			StrategyOptions options) {
		Util.checkNull(publication);
		Util.checkNull(publication.getItemName().getRendezvousId(),
				"no rid in name");
		Util.checkNull(publication.getItemName().getScopeId(),
				"no scope in name");

		byte[] str = publication.getItemName().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte[] opt_arr = null;
		if(options != null && options.getSerializedSize() > 0){
			opt_arr = options.toByteArray();
		}
		
		byte[] data = publication.getData();
		baWrapper.publishData(str, strategyValue, opt_arr, data);
	}
	
	public void publishData(byte[] id, byte[] data, Strategy strategy) {
		publishData(id, data, strategy, null);
	}

	public void publishData(byte[] id, byte[] data, Strategy strategy,
			StrategyOptions options) {
		byte[] opt_arr = null;
		if(options != null && options.getSerializedSize() > 0){
			opt_arr = options.toByteArray();
		}
		baWrapper.publishData(id, strategy.byteValue(), opt_arr, data);
	}
	
	public void publishData(byte[] id, ByteBuffer buffer, Strategy strategy) {
		publishData(id, buffer, strategy, null);
	}

	public void publishData(byte[] id, ByteBuffer buffer, Strategy strategy,
			StrategyOptions options) {
		if (!buffer.isDirect()) {
			throw new IllegalArgumentException(
					"Bytebuffer must be direct. Use ByteBuffer.allocateDirect() first");
		}

		byte[] opt_arr = null;
		if(options != null && options.getSerializedSize() > 0){
			opt_arr = options.toByteArray();
		}
		baWrapper.publishData(id, strategy.byteValue(), opt_arr, buffer);
	}

	public Event getNextEvent() {
		return baWrapper.getNextEventDirect();
	}

	public Event getNextEvent(long timeout, TimeUnit units)
			throws InterruptedException {
		throw new UnsupportedOperationException("not implemented yet");
	}

	public void disconnect() {
		if (!closed) {
			closed = true;
			baWrapper.close();
		}
	}

	@Override
	protected void finalize() throws Throwable {
		disconnect();
		super.finalize();
	}
}
