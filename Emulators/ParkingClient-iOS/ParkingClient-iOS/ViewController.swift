//
//  ViewController.swift
//  ParkingClient-iOS
//
//  Created by Михаил Конюхов on 25.12.2025.
//

import UIKit

class ViewController: UIViewController {
    
    private let barrierViewModel = BarrierViewModel()
    
    private var stackView: UIStackView?
    private var statusLabel: UILabel?
    private var statusBarrierLabel: UILabel?
    private var stateBarrierLabel: UILabel?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupViews()
        setupCallbacks()
        barrierViewModel.connect()
    }
    
    private func setupViews() {
        stackView = UIStackView()
        stackView?.translatesAutoresizingMaskIntoConstraints = false
        guard let stackView else { return }
        stackView.axis = .vertical
        stackView.distribution = .fillEqually
        view.addSubview(stackView)
        NSLayoutConstraint.activate([
            stackView.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 16.0),
            stackView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -16.0),
            stackView.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 16.0),
            stackView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: -16.0),
        ])
        setupStatusLabel(false)
        stackView.addArrangedSubview(setupBarrierStatusLabel())
        stackView.addArrangedSubview(setupBarrierStateLabel())
    }
    
    private func setupStatusLabel(_ isConnected: Bool) {
        statusLabel = UILabel()
        statusLabel?.font = UIFont.systemFont(ofSize: 20.0, weight: .semibold)
        statusLabel?.text = isConnected ? "WebSocket: Подключен" : "WebSocket: Отключен"
        statusLabel?.textColor = isConnected ? .systemGreen : .systemRed
        stackView?.addArrangedSubview(statusLabel!)
    }
    
    private func setupBarrierStatusLabel() -> UIView {
        let stackView = UIStackView()
        stackView.axis = .horizontal
        
        let headerLabel = UILabel()
        headerLabel.text = "Состояние шлагбаума: "
        stackView.addArrangedSubview(headerLabel)
        
        statusBarrierLabel = UILabel()
        statusBarrierLabel?.text = "Закрыт"
        stackView.addArrangedSubview(statusBarrierLabel!)
        
        return stackView
    }
    
    private func setupBarrierStateLabel() -> UIView {
        let stackView = UIStackView()
        stackView.axis = .horizontal
        
        let headerLabel = UILabel()
        headerLabel.text = "Положение стрелы: "
        stackView.addArrangedSubview(headerLabel)
        
        stateBarrierLabel = UILabel()
        stateBarrierLabel?.text = "0"
        stackView.addArrangedSubview(stateBarrierLabel!)
        
        return stackView
    }
    
    private func setupCallbacks() {
        barrierViewModel.onMessagesUpdated = { [weak self] in
            guard let self else { return }
            statusBarrierLabel?.text = barrierViewModel.isOpen ? "Открыт" : "Закрыт"
            stateBarrierLabel?.text = "\(barrierViewModel.position) %"
        }
        
        barrierViewModel.onStatusChanged = { [weak self] isConnected in
            guard let self else { return }
            statusLabel?.text = isConnected ? "WebSocket: Подключен" : "WebSocket: Отключен"
            statusLabel?.textColor = isConnected ? .systemGreen : .systemRed
        }
    }

}

