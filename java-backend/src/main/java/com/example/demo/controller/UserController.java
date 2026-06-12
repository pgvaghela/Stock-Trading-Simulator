package com.example.demo.controller;

import com.example.demo.entity.User;
import com.example.demo.repository.UserRepository;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/users")
public class UserController {
    private final UserRepository userRepo;

    public UserController(UserRepository userRepo) {
        this.userRepo = userRepo;
    }

    @PostMapping
    public User createUser(@RequestBody(required = false) User user) {
        if (user == null) user = new User();
        if (user.getUsername() == null) user.setUsername("user" + System.currentTimeMillis());
        if (user.getPasswordHash() == null) user.setPasswordHash("demo");
        // Optionally set default username, password, etc.
        return userRepo.save(user);
    }
} 