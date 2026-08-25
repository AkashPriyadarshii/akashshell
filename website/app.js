// Magnetic Hover Effect for Terminal Window
const terminal = document.getElementById('hero-terminal');

if (terminal) {
    document.addEventListener('mousemove', (e) => {
        const xAxis = (window.innerWidth / 2 - e.pageX) / 50;
        const yAxis = (window.innerHeight / 2 - e.pageY) / 50;
        
        // Prevent extreme tilting
        const constrainedX = Math.max(-10, Math.min(10, xAxis));
        const constrainedY = Math.max(-10, Math.min(10, yAxis));

        terminal.style.transform = `rotateY(${constrainedX}deg) rotateX(${constrainedY}deg) translateZ(10px)`;
    });

    // Reset on mouse leave window
    document.addEventListener('mouseleave', () => {
        terminal.style.transform = `rotateY(0deg) rotateX(0deg) translateZ(0px)`;
    });
}

// Intersection Observer for Scroll Animations (Fade Up / Fade In)
const observerOptions = {
    root: null,
    rootMargin: '0px',
    threshold: 0.1
};

const observer = new IntersectionObserver((entries, observer) => {
    entries.forEach(entry => {
        if (entry.isIntersecting) {
            entry.target.classList.add('is-visible');
            observer.unobserve(entry.target); // Only animate once
        }
    });
}, observerOptions);

document.querySelectorAll('.fade-up, .fade-in, .bento-anim').forEach((el) => {
    if (el.classList.contains('bento-anim')) {
        el.classList.add('fade-up');
    }
    observer.observe(el);
});

// Typing effect loop
const typingText = document.querySelector('.typing-text');
const prompts = [
    "cargo build --release",
    "akashshell --version",
    "ls | sort-by size"
];
let promptIndex = 0;
let charIndex = 0;
let isDeleting = false;

function typeLoop() {
    const currentPrompt = prompts[promptIndex];
    
    if (isDeleting) {
        typingText.textContent = currentPrompt.substring(0, charIndex - 1) + "_";
        charIndex--;
    } else {
        typingText.textContent = currentPrompt.substring(0, charIndex + 1) + "_";
        charIndex++;
    }
    
    let typeSpeed = isDeleting ? 30 : 100;
    
    if (!isDeleting && charIndex === currentPrompt.length) {
        typeSpeed = 2000; // Pause at end
        isDeleting = true;
    } else if (isDeleting && charIndex === 0) {
        isDeleting = false;
        promptIndex = (promptIndex + 1) % prompts.length;
        typeSpeed = 500; // Pause before new word
    }
    
    setTimeout(typeLoop, typeSpeed);
}

// Start typing effect
if (typingText) {
    setTimeout(typeLoop, 1000);
}
