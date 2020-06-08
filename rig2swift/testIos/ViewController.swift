import UIKit

class ViewController: UIViewController {

   var label = UILabel()
   
   override func viewDidLoad() {
      super.viewDidLoad()
      view.backgroundColor = .white
      
      // Provide a basic label to see text output
      label.frame = view.frame
      label.backgroundColor = .gray
      label.numberOfLines = 0
      view.addSubview( label )
   }


}
