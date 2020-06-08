import UIKit
import SwiftUI
import rig2swift

class SceneDelegate: UIResponder, UIWindowSceneDelegate {

   var window: UIWindow?
   var rig: rig2swift?
   var log : String = ""

   func updateLabel( _ text : String )
   {
      //print( text )
      
      log.append( text + "\n" )
      DispatchQueue.main.async {
         let viewController = (self.window!.rootViewController as! ViewController)
         viewController.label.text = self.log
         viewController.label.sizeToFit()
      }
   }
   func scene(_ scene: UIScene, willConnectTo session: UISceneSession, options connectionOptions: UIScene.ConnectionOptions) {
      // Use this method to optionally configure and attach the UIWindow `window` to the provided UIWindowScene `scene`.
      // If using a storyboard, the `window` property will automatically be initialized and attached to the scene.
      // This delegate does not imply the connecting scene or session are new (see `application:configurationForConnectingSceneSession` instead)
      
      // Use a UIHostingController as window root view controller
      let viewController = ViewController()
      if let windowScene = scene as? UIWindowScene {
          let window = UIWindow(windowScene: windowScene)
          window.rootViewController = viewController
          self.window = window
          window.makeKeyAndVisible()
      }
      
      // Load the first JSON file we have in our bundle
      let filename = Bundle.main.path( forResource: nil, ofType: "json" )
      
      rig = rig2swift()
      rig?.initialize()
      
      rig?.OnError = { (rigId, errorCode, description) in
         let errorCodeStr = String(errorCode.rawValue)
         self.updateLabel( rigId + " " + errorCodeStr + ": " + description )
         
         // Add a helpeful iOS-specific hint here
         if errorCode == .BAD_PATH {
            self.updateLabel( "Ensure you have added a rig file to your application bundle" )
         }
      }
      
      var characters:[String: Int] = [:]
      var globalStartFrame = 9999999
      var globalEndFrame = 0
      rig?.OnBounds = { (rigId, startFrame, endFrame) in
         
         // Set the global start/end frame, if necessary
         if startFrame < globalStartFrame {
            globalStartFrame = startFrame
         }
         if endFrame > globalEndFrame {
            globalEndFrame = endFrame
         }
      
         characters[rigId] = 0
      }
      
      rig?.OnFrame = { ( rigId,
         frameTimestamp,
         locationXYZ,
         boneRotations,
         boneLengths,
         boneOffsets) in
         
         characters[ rigId ]! += 1
         
         // Test the arrays by accessing every element
         for i in 0..<3 {
            let value = locationXYZ[i]
            if abs(value) > 1000000 {
               self.updateLabel( "'" + rigId + "' frame " + String(frameTimestamp) +
                  ", location: Unreasonable value " + String(value) )
               return
            }
         }
            
         if ( boneRotations != nil )
         {
            for i in 0..<boneRotations!.count {
               let value = boneRotations![i]
               if abs(value) > 1 {
                  self.updateLabel( "'" + rigId + "' frame " + String(frameTimestamp) +
                     ", rotation: Unreasonable value " + String(value) )
                  return
               }
            }
         }
         
         if ( boneLengths != nil )
         {
            for i in 0..<boneLengths!.count {
               let value = boneLengths![i]
               if value > 1000 || value <= 0.01 {
                  self.updateLabel( "'" + rigId + "' frame " + String(frameTimestamp) +
                     ", length: Unreasonable value " + String(value) )
                  return
               }
            }
         }
         
         if ( boneOffsets != nil )
         {
         for i in 0..<boneOffsets!.count {
               let value = boneOffsets![i]
               if abs(value) > 100 {
                  self.updateLabel( "'" + rigId + "' frame " + String(frameTimestamp) +
                     ", offset: Unreasonable value " + String(value) )
                  return
               }
            }
         }
      }
      
      rig?.read( url:filename ?? "" )
      
      // Get some info
      let fps = rig?.getInfo( attribute:"fps" )
      self.updateLabel( "FPS=" + fps! )
   
      // Print a summary
      if characters.count > 0 {
         var averageNumFramesPerCharacter = 0.0
         for (_, numFrames ) in characters {
            averageNumFramesPerCharacter += Double(numFrames)
         }
         averageNumFramesPerCharacter /= Double(characters.count)
         updateLabel( "------------------------------------------------------" )
         updateLabel( "Approx " + String(Int(averageNumFramesPerCharacter)) + " frames per character, " +
            String(characters.count) + " characters, " +
            "bounds " + String(globalStartFrame) + "->" + String(globalEndFrame) )
      }
      
      rig?.uninitialize()
      
      
   }

   func sceneDidDisconnect(_ scene: UIScene) {
      // Called as the scene is being released by the system.
      // This occurs shortly after the scene enters the background, or when its session is discarded.
      // Release any resources associated with this scene that can be re-created the next time the scene connects.
      // The scene may re-connect later, as its session was not neccessarily discarded (see `application:didDiscardSceneSessions` instead).
   }

   func sceneDidBecomeActive(_ scene: UIScene) {
      // Called when the scene has moved from an inactive state to an active state.
      // Use this method to restart any tasks that were paused (or not yet started) when the scene was inactive.
   }

   func sceneWillResignActive(_ scene: UIScene) {
      // Called when the scene will move from an active state to an inactive state.
      // This may occur due to temporary interruptions (ex. an incoming phone call).
   }

   func sceneWillEnterForeground(_ scene: UIScene) {
      // Called as the scene transitions from the background to the foreground.
      // Use this method to undo the changes made on entering the background.
   }

   func sceneDidEnterBackground(_ scene: UIScene) {
      // Called as the scene transitions from the foreground to the background.
      // Use this method to save data, release shared resources, and store enough scene-specific state information
      // to restore the scene back to its current state.
   }


}

