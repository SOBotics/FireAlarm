//
//  BackgroundTasks.swift
//  FireAlarm
//
//  Created by NobodyNada on 4/18/17.
//
//

import Foundation
import Dispatch

public final class BackgroundTask: Hashable {
	public private(set) var isCancelled = false
	public private(set) var isScheduled = false
	
	private(set) var manager: BackgroundTaskManager!
	
	public var run: ((BackgroundTask) -> ())?
	
	public func onRun(_ block: @escaping (BackgroundTask) -> ()) {
		run = block
	}
	
	
	
	public var interval: TimeInterval?
	
	public init(interval: TimeInterval? = nil, run: @escaping (BackgroundTask) -> ()) {
		self.interval = interval
		self.run = run
	}
	
	
	
	public func schedule(manager: BackgroundTaskManager) {
		if isScheduled { return }
		self.manager = manager
		
		isScheduled = true
		isCancelled = false
		
		if let interval = interval {
			
			manager.queue.asyncAfter(deadline: DispatchTime.now() + interval) {
				if self.isCancelled { return }
				
				self.run?(self)
				self.isScheduled = false
				
				if self.isCancelled { return }
				self.schedule(manager: manager)
			}
			
		} else {
			manager.queue.async {
				if self.isCancelled { return }
				
				self.run?(self)
				self.isScheduled = false
			}
		}
	}
	
	public func cancel() {
		if isCancelled { return }
		
		isCancelled = true
		isScheduled = false
		if let manager = manager {
			manager.tasks = manager.tasks.filter { $0 != self }
		}
	}
	
	public static func ==(lhs: BackgroundTask, rhs: BackgroundTask) -> Bool {
		return lhs === rhs
	}
	
	public var hashValue: Int {
		return Unmanaged.passUnretained(self).toOpaque().hashValue
	}
}

open class BackgroundTaskManager {
	open var tasks = [BackgroundTask]() {
		didSet {
			//Schedule all tasks which have not been scheduled.
			for task in tasks {
				if !task.isScheduled && !task.isCancelled {
					task.schedule(manager: self)
				}
			}
			
			//Cancel all tasks which were removed from the array.
			for task in Set<BackgroundTask>(oldValue).subtracting(tasks) {
				task.cancel()
			}
		}
	}
	
	public let queue = DispatchQueue(label: "Background Tasks", attributes: .concurrent)
	
	public static var shared = BackgroundTaskManager()
}
