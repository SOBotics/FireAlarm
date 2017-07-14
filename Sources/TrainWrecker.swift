//
//  TrainWrecker.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/7/17.
//
//

import Foundation
import SwiftChatSE

let editDistanceWeight: Float = 1.6    //How quickly edit distance causes confidence to drop.
let confidenceWeight: Float = 1.1      //How quickly confidence increases the chance of wrecking a train.
let minThreshold = 32
let maxThreshold = 96

let trains = [  //From https://github.com/Tiny-Giant/myuserscripts/blob/master/Trainwreck.user.js
    "https://upload.wikimedia.org/wikipedia/commons/1/19/Train_wreck_at_Montparnasse_1895.jpg",
    "https://s3.amazonaws.com/img.ultrasignup.com/events/raw/6a76f4a3-4ad2-4ae2-8a3b-c092e85586af.jpg",
    "https://kassontrainwreck.files.wordpress.com/2015/03/cropped-trainwreck.jpg",
    "http://www.ncbam.org/images/photos/train-wreck.jpg",
    "http://oralfixationshow.com/wp-content/uploads/2014/09/train-wreck.jpg",
    "http://experiencedynamics.blogs.com/.a/6a00d8345a66bf69e201901ed419a4970b-pi",
    "https://timedotcom.files.wordpress.com/2015/05/150513-1943-train-wreck-02.jpg?quality=75&strip=color&w=573",
    "http://static6.businessinsider.com/image/5554e92369bedd8f33c45a0d/heres-everything-we-know-about-the-amtrak-train-wreck-in-philadelphia.jpg",
    "http://goldsilverworlds.com/wp-content/uploads/2015/07/trainwreck.jpg",
    "http://allthingsd.com/files/2012/06/trainwreck.jpg",
    "http://sailinganarchy.com/wp-content/uploads/2015/09/trainwreck.jpg",
    "http://iamphildodd.com/wp-content/uploads/2013/12/trainwreck1.jpg",
    "http://cdn.theatlantic.com/assets/media/img/posts/2013/11/758px_Train_Wreck_1922/56ae9c9fc.jpg",
    "http://static.messynessychic.com/wp-content/uploads/2012/10/trainwreck.jpg",
    "http://www.baycolonyrailtrail.org/gallery2/d/1282-3/trainwreck.jpg",
    "http://trainwreckwinery.com/wp-content/uploads/2012/05/trainwreckTHEN.jpg",
    "http://www.skvarch.com/images/trains/trainwreck.jpg",
    "http://conselium.com/wp-content/uploads/train-wreck.jpg",
    "http://imgs.sfgate.com/blogs/images/sfgate/bmangan/2010/10/18/trainwreck.jpg",
    "https://img1.etsystatic.com/043/0/7724935/il_fullxfull.613833437_37a5.jpg",
    "http://ncpedia.org/sites/default/files//bostian_wreck.jpg",
    "https://upload.wikimedia.org/wikipedia/commons/9/9f/NewMarketTrainWreck.jpg",
    "https://ethicsalarms.files.wordpress.com/2015/04/train-wrecks-accidents.jpg",
    "http://offbeatoregon.com/Images/H1002b_General/BridgeWreck1800.jpg",
    "http://www3.gendisasters.com/files/newphotos/Naperville%20IL%20Train%20Wreck%204-26-1946.JPG",
    "http://static01.nyt.com/images/2011/07/25/world/25china-span/25china-span-articleLarge.jpg",
    "http://shorespeak.com/blog/wp-content/uploads/2011/01/train_wreck_2.jpg",
    "http://www.cfm-fmh.org/files/QuickSiteImages/MuseumPhotos/Train_Wreck.jpg",
    "http://www.circusesandsideshows.com/images/algbarnestrainwrecklarge.jpg",
    "http://www.scitechantiques.com/trainwreck/trainwreck.jpg",
    "http://www3.gendisasters.com/files/newphotos/nj-woodbridge-trainwreck3r.jpg",
    "http://travel.gunaxin.com/wp-content/uploads/2010/07/Ep9_TrainWreck.jpg"
]

class TrainWrecker {
    let room: ChatRoom
    init(room: ChatRoom) {
        self.room = room
    }
    
    var lastMessage: String? = nil
    var confidence: Float = 1   //How confindent we are that a train is in progress,
    //with 1 being the lowest possible confidence.
    
    func randomNumber(min: Int, max: Int) -> Int {
        #if os(Linux)
            return (Int(rand()) % (max + 1 - min)) + min
        #else
            return Int(arc4random_uniform(UInt32(max + 1 - min))) + min
        #endif
    }
    
    func process(message: ChatMessage) {
        defer { lastMessage = message.content }
        guard let last = lastMessage else { return }
        
        let editDistance = Levenshtein.distanceBetween(message.content, and: last)
        confidence = max(1, confidence * (-pow(editDistanceWeight, Float(editDistance)) + 6))
        
        let threshold = randomNumber(min: minThreshold, max: maxThreshold)
        
        if Float(threshold) < pow(confidence, confidenceWeight) {
            print("Wrecking train in \(room.roomID) (confidence \(confidence)); threshold \(threshold).")
            let train = trains[randomNumber(min: 0, max: trains.count - 1)]
            room.postMessage("[#RekdTrain](\(train))")
        }
    }
}
