module atk.wrapper {
    exports org.GNOME.Accessibility;
    requires java.desktop;
    provides javax.accessibility.AccessibilityProvider
        with org.GNOME.Accessibility.AtkProvider;
}
